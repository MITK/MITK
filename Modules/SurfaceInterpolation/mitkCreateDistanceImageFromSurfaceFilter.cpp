/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkCreateDistanceImageFromSurfaceFilter.h"
#include "mitkImageCast.h"

mitk::CreateDistanceImageFromSurfaceFilter::CreateDistanceImageFromSurfaceFilter()
{
  m_DistanceImageVolume = 50000;
  this->m_UseProgressBar = false;
  this->m_ProgressStepSize = 5;

  mitk::Image::Pointer output = mitk::Image::New();
  this->SetNthOutput(0, output.GetPointer());
}


mitk::CreateDistanceImageFromSurfaceFilter::~CreateDistanceImageFromSurfaceFilter()
{
}

void mitk::CreateDistanceImageFromSurfaceFilter::GenerateData()
{
  //First of all we have to build the equation-system from the existing contour-edge-points
  this->CreateSolutionMatrixAndFunctionValues();

  if (this->m_UseProgressBar)
    mitk::ProgressBar::GetInstance()->Progress(1);

  m_Weights = m_SolutionMatrix.partialPivLu().solve(m_FunctionValues);

  if (this->m_UseProgressBar)
    mitk::ProgressBar::GetInstance()->Progress(2);

  //The last step is to create the distance map with the interpolated distance function
  this->CreateDistanceImage();

  if (this->m_UseProgressBar)
    mitk::ProgressBar::GetInstance()->Progress(2);

  m_Centers.clear();
  m_Normals.clear();
}

void mitk::CreateDistanceImageFromSurfaceFilter::CreateSolutionMatrixAndFunctionValues()
{
  unsigned int numberOfInputs = this->GetNumberOfIndexedInputs();

  if (numberOfInputs == 0)
  {
    MITK_ERROR << "mitk::CreateDistanceImageFromSurfaceFilter: No input available. Please set an input!" << std::endl;
    itkExceptionMacro("mitk::CreateDistanceImageFromSurfaceFilter: No input available. Please set an input!");
    return;
  }

  //First of all we have to extract the nomals and the surface points.
  //Duplicated points can be eliminated

  Surface* currentSurface;
  vtkSmartPointer<vtkPolyData> polyData;
  vtkSmartPointer<vtkDoubleArray> currentCellNormals;
  vtkSmartPointer<vtkCellArray> existingPolys;
  vtkSmartPointer<vtkPoints> existingPoints;

  double p[3];
  PointType currentPoint;
  PointType normal;

  for (unsigned int i = 0; i < numberOfInputs; i++)
  {
    currentSurface = const_cast<Surface*>( this->GetInput(i) );
    polyData = currentSurface->GetVtkPolyData();

    if (polyData->GetNumberOfPolys() == 0)
    {
      MITK_INFO << "mitk::CreateDistanceImageFromSurfaceFilter: No input-polygons available. Please be sure the input surface consists of polygons!" << std::endl;
    }

    currentCellNormals = vtkDoubleArray::SafeDownCast(polyData->GetCellData()->GetNormals());

    existingPolys = polyData->GetPolys();

    existingPoints = polyData->GetPoints();

    existingPolys->InitTraversal();

    vtkIdType* cell (NULL);
    vtkIdType cellSize (0);

    for( existingPolys->InitTraversal(); existingPolys->GetNextCell(cellSize, cell);)
    {
      for ( vtkIdType j = 0; j < cellSize; j++ )
      {
        existingPoints->GetPoint(cell[j], p);

        currentPoint.copy_in(p);

        int count = std::count(m_Centers.begin() ,m_Centers.end(),currentPoint);

        if (count == 0)
        {
          double currentNormal[3];
          currentCellNormals->GetTuple(cell[j], currentNormal);

          normal.copy_in(currentNormal);

          m_Normals.push_back(normal);

          m_Centers.push_back(currentPoint);
        }

      }//end for all points
    }//end for all cells
  }//end for all outputs

  //For we can now calculate the exact size of the centers we initialize the data structures
  unsigned int numberOfCenters = m_Centers.size();
  m_Centers.reserve(numberOfCenters*3);

  m_FunctionValues.resize(numberOfCenters*3);

  m_FunctionValues.fill(0);

  //Create inner points
  for (unsigned int i = 0; i < numberOfCenters; i++)
  {
    currentPoint = m_Centers.at(i);
    normal = m_Normals.at(i);

    currentPoint[0] = currentPoint[0] - normal[0];
    currentPoint[1] = currentPoint[1] - normal[1];
    currentPoint[2] = currentPoint[2] - normal[2];

    m_Centers.push_back(currentPoint);

    m_FunctionValues[numberOfCenters+i] = -1;
  }

  //Create outer points
  for (unsigned int i = 0; i < numberOfCenters; i++)
  {
    currentPoint = m_Centers.at(i);
    normal = m_Normals.at(i);

    currentPoint[0] = currentPoint[0] + normal[0];
    currentPoint[1] = currentPoint[1] + normal[1];
    currentPoint[2] = currentPoint[2] + normal[2];

    m_Centers.push_back(currentPoint);

    m_FunctionValues[numberOfCenters*2+i] = 1;
  }

  //Now we have created all centers and all function values. Next step is to create the solution matrix
  numberOfCenters = m_Centers.size();

  m_SolutionMatrix.resize(numberOfCenters, numberOfCenters);

  m_Weights.resize(numberOfCenters);

  PointType p1;
  PointType p2;
  double norm;

  for (unsigned int i = 0; i < numberOfCenters; i++)
  {
    for (unsigned int j = 0; j < numberOfCenters; j++)
    {
      //Calculate the RBF value. Currently using Phi(r) = r with r is the euclidian distance between two points
      p1 = m_Centers.at(i);
      p2 = m_Centers.at(j);
      p1 = p1 - p2;
      norm = p1.two_norm();
      m_SolutionMatrix(i,j) = norm;
    }
  }
}

void mitk::CreateDistanceImageFromSurfaceFilter::CreateDistanceImage()
{
  DistanceImageType::Pointer distanceImg = DistanceImageType::New();

  // Determine the bounds of the input points in index- and world-coordinates
  DistanceImageType::PointType minPointInWorldCoordinates, maxPointInWorldCoordinates;
  DistanceImageType::IndexType minPointInIndexCoordinates, maxPointInIndexCoordinates;

  DetermineBounds( minPointInWorldCoordinates, maxPointInWorldCoordinates,
                   minPointInIndexCoordinates, maxPointInIndexCoordinates );


  // Calculate the extent of the region that contains all given points in MM.
  // To do this, we take the difference between the maximal and minimal
  // index-coordinates (must not be less than 1) and multiply it with the
  // spacing of the reference-image.
  Vector3D extentMM;
  for (unsigned int dim = 0; dim < 3; ++dim)
  {
    extentMM[dim] = (int)
      (
                    (std::max( std::abs(maxPointInIndexCoordinates[dim] - minPointInIndexCoordinates[dim]),
                              (DistanceImageType::IndexType::IndexValueType) 1
                            ) + 1.0) // (max-index - min-index)+1 because the pixels between index 3 and 5 cover 2+1=3 pixels (pixel 3,4, and 5)
                    * m_ReferenceImage->GetSpacing()[dim]
      ) + 1; // (int) ((...) + 1) -> we round up to the next BIGGER int value
  }

  /*
  * Now create an empty distance image. The create image will always have the same sizeOfRegion, independent from
  * the original image (e.g. always consists of 500000 pixels) and will have an isotropic spacing.
  * The spacing is calculated like the following:
  * The image's volume = 500000 Pixels = extentX*spacing*extentY*spacing*extentZ*spacing
  * So the spacing is: spacing = ( 500000 / extentX*extentY*extentZ )^(1/3)
  */
  double basis = (extentMM[0]*extentMM[1]*extentMM[2]) / m_DistanceImageVolume;
  double exponent = 1.0/3.0;
  double distImgSpacing = pow(basis, exponent);
  int tempSpacing = (distImgSpacing+0.05)*10;
  m_DistanceImageSpacing = (double)tempSpacing/10.0;

  // calculate the number of pixels of the distance image for each direction
  unsigned int numberOfXPixel = extentMM[0] / m_DistanceImageSpacing;
  unsigned int numberOfYPixel = extentMM[1] / m_DistanceImageSpacing;
  unsigned int numberOfZPixel = extentMM[2] / m_DistanceImageSpacing;

  // We increase the sizeOfRegion by 4 as we decrease the origin by 2 later.
  // This expansion of the region is necessary to achieve a complete
  // interpolation.
  DistanceImageType::SizeType sizeOfRegion;
  sizeOfRegion[0] = numberOfXPixel + 4;
  sizeOfRegion[1] = numberOfYPixel + 4;
  sizeOfRegion[2] = numberOfZPixel + 4;

  // The region starts at index 0,0,0
  DistanceImageType::IndexType initialOriginAsIndex;
  initialOriginAsIndex.Fill(0);

  DistanceImageType::PointType originAsWorld = minPointInWorldCoordinates;

  DistanceImageType::RegionType lpRegion;
  lpRegion.SetSize(sizeOfRegion);
  lpRegion.SetIndex(initialOriginAsIndex);

  // We initialize the itk::Image with
  //  * origin and direction to have it correctly placed and rotated in the world
  //  * the largest possible region to set the extent to be calculated
  //  * the isotropic spacing that we have calculated above
  distanceImg->SetOrigin( originAsWorld );
  distanceImg->SetDirection( m_ReferenceImage->GetDirection() );
  distanceImg->SetRegions( lpRegion );
  distanceImg->SetSpacing( m_DistanceImageSpacing );
  distanceImg->Allocate();

  //First of all the image is initialized with the value 10 for each pixel
  distanceImg->FillBuffer(10);

  // Now we move the origin of the distanceImage 2 index-Coordinates
  // in all directions
  DistanceImageType::IndexType originAsIndex;
  distanceImg->TransformPhysicalPointToIndex( originAsWorld, originAsIndex );
  originAsIndex[0] -= 2;
  originAsIndex[1] -= 2;
  originAsIndex[2] -= 2;
  distanceImg->TransformIndexToPhysicalPoint( originAsIndex, originAsWorld );
  distanceImg->SetOrigin( originAsWorld );

  /*
  * Now we must calculate the distance for each pixel. But instead of calculating the distance value
  * for all of the image's pixels we proceed similar to the region growing algorithm:
  *
  * 1. Take the first pixel from the narrowband_point_list and calculate the distance for each neighbor (6er)
  * 2. If the current index's distance value is below a certain threshold push it into the list
  * 3. Next iteration take the next index from the list and originAsIndex with 1. again
  *
  * This is done until the narrowband_point_list is empty.
  */
  std::queue<DistanceImageType::IndexType> narrowbandPoints;
  PointType currentPoint = m_Centers.at(0);
  double distance = this->CalculateDistanceValue(currentPoint);

  // create itk::Point from vnl_vector
  DistanceImageType::PointType currentPointAsPoint;
  currentPointAsPoint[0] = currentPoint[0];
  currentPointAsPoint[1] = currentPoint[1];
  currentPointAsPoint[2] = currentPoint[2];

  // Transform the input point in world-coordinates to index-coordinates
  DistanceImageType::IndexType currentIndex;
  distanceImg->TransformPhysicalPointToIndex( currentPointAsPoint, currentIndex );

  assert( lpRegion.IsInside(currentIndex) ); // we are quite certain this should hold

  narrowbandPoints.push(currentIndex);
  distanceImg->SetPixel(currentIndex, distance);

  NeighborhoodImageIterator::RadiusType radius;
  radius.Fill(1);
  NeighborhoodImageIterator nIt(radius, distanceImg, distanceImg->GetLargestPossibleRegion());
  unsigned int relativeNbIdx[] = {4, 10, 12, 14, 16, 22};

  bool isInBounds = false;
  while ( !narrowbandPoints.empty() )
  {

    nIt.SetLocation(narrowbandPoints.front());
    narrowbandPoints.pop();

    unsigned int* relativeNb = &relativeNbIdx[0];
    for (int i = 0; i < 6; i++)
    {
      nIt.GetPixel(*relativeNb, isInBounds);
      if( isInBounds && nIt.GetPixel(*relativeNb) == 10)
      {
        currentIndex = nIt.GetIndex(*relativeNb);

        // Transform the currently checked point from index-coordinates to
        // world-coordinates
        distanceImg->TransformIndexToPhysicalPoint( currentIndex, currentPointAsPoint );

        // create a vnl_vector
        currentPoint[0] = currentPointAsPoint[0];
        currentPoint[1] = currentPointAsPoint[1];
        currentPoint[2] = currentPointAsPoint[2];

        // and check the distance
        distance = this->CalculateDistanceValue(currentPoint);
        if ( (std::fabs(distance)) <= m_DistanceImageSpacing )
        {
          nIt.SetPixel(*relativeNb, distance);
          narrowbandPoints.push(currentIndex);
        }
      }
      relativeNb++;
    }
  }


  ImageIterator imgRegionIterator (distanceImg, distanceImg->GetLargestPossibleRegion());
  imgRegionIterator.GoToBegin();

  double prevPixelVal = 1;

  DistanceImageType::IndexType _size;
  _size.Fill(-1);
  _size += sizeOfRegion;

  //Set every pixel inside the surface to -10 except the edge point (so that the received surface is closed)
  while (!imgRegionIterator.IsAtEnd()) {

    if ( imgRegionIterator.Get() == 10 && prevPixelVal < 0 )
    {

      while (imgRegionIterator.Get() == 10)
      {
        if (imgRegionIterator.GetIndex()[0] == _size[0] || imgRegionIterator.GetIndex()[1] == _size[1] || imgRegionIterator.GetIndex()[2] == _size[2]
            || imgRegionIterator.GetIndex()[0] == 0U || imgRegionIterator.GetIndex()[1] == 0U || imgRegionIterator.GetIndex()[2] == 0U )
        {
          imgRegionIterator.Set(10);
          prevPixelVal = 10;
          ++imgRegionIterator;
          break;
        }
        else
        {
          imgRegionIterator.Set(-10);
          ++imgRegionIterator;
          prevPixelVal = -10;
        }

      }

    }
    else if (imgRegionIterator.GetIndex()[0] == _size[0] || imgRegionIterator.GetIndex()[1] == _size[1] || imgRegionIterator.GetIndex()[2] == _size[2]
             || imgRegionIterator.GetIndex()[0] == 0U || imgRegionIterator.GetIndex()[1] == 0U || imgRegionIterator.GetIndex()[2] == 0U)

    {
      imgRegionIterator.Set(10);
      prevPixelVal = 10;
      ++imgRegionIterator;
    }
    else
    {
      prevPixelVal = imgRegionIterator.Get();
      ++imgRegionIterator;
    }
  }

  Image::Pointer resultImage = this->GetOutput();

  // Cast the created distance-Image from itk::Image to the mitk::Image
  // that is our output.
  CastToMitkImage(distanceImg, resultImage);
}

void mitk::CreateDistanceImageFromSurfaceFilter::FillImageRegion(DistanceImageType::RegionType reqRegion,
                                                                 DistanceImageType::PixelType pixelValue, DistanceImageType::Pointer image)
{
  image->SetRequestedRegion(reqRegion);
  ImageIterator it (image, image->GetRequestedRegion());
  while (!it.IsAtEnd())
  {
    it.Set(pixelValue);
    ++it;
  }

}


double mitk::CreateDistanceImageFromSurfaceFilter::CalculateDistanceValue(PointType p)
{
  double distanceValue (0);
  PointType p1;
  PointType p2;
  double norm;

  CenterList::iterator centerIter;

  unsigned int count (0);
  for ( centerIter=m_Centers.begin();
    centerIter!=m_Centers.end();
    centerIter++)
  {
    p1 = *centerIter;
    p2 = p-p1;
    norm = p2.two_norm();
    distanceValue = distanceValue + (norm * m_Weights[count]);
    ++count;
  }
  return distanceValue;
}

void mitk::CreateDistanceImageFromSurfaceFilter::GenerateOutputInformation()
{
}

void mitk::CreateDistanceImageFromSurfaceFilter::PrintEquationSystem()
{
  std::stringstream out;
  out<<"Nummber of rows: "<<m_SolutionMatrix.rows()<<" ****** Number of columns: "<<m_SolutionMatrix.cols()<<endl;
  out<<"[ ";
  for (int i = 0; i < m_SolutionMatrix.rows(); i++)
  {
    for (int j = 0; j < m_SolutionMatrix.cols(); j++)
    {
      out<<m_SolutionMatrix(i,j)<<"   ";
    }
    out<<";"<<endl;
  }
  out<<" ]\n\n\n";

  for (unsigned int i = 0; i < m_Centers.size(); i++)
  {
    out<<m_Centers.at(i)<<";"<<endl;
  }
  std::cout<<"Equation system: \n\n\n"<<out.str();
}



void mitk::CreateDistanceImageFromSurfaceFilter::SetInput( const mitk::Surface* surface )
{
    this->SetInput( 0, const_cast<mitk::Surface*>( surface ) );
}


void mitk::CreateDistanceImageFromSurfaceFilter::SetInput( unsigned int idx, const mitk::Surface* surface )
{
    if ( this->GetInput(idx) != surface )
    {
        this->SetNthInput( idx, const_cast<mitk::Surface*>( surface ) );
        this->Modified();
    }
}


const mitk::Surface* mitk::CreateDistanceImageFromSurfaceFilter::GetInput()
{
    if (this->GetNumberOfIndexedInputs() < 1)
        return NULL;

    return static_cast<const mitk::Surface*>(this->ProcessObject::GetInput(0));
}


const mitk::Surface* mitk::CreateDistanceImageFromSurfaceFilter::GetInput( unsigned int idx)
{
    if (this->GetNumberOfIndexedInputs() < 1)
        return NULL;

    return static_cast<const mitk::Surface*>(this->ProcessObject::GetInput(idx));
}

void mitk::CreateDistanceImageFromSurfaceFilter::RemoveInputs(mitk::Surface* input)
{
  DataObjectPointerArraySizeType nb = this->GetNumberOfIndexedInputs();

  for(DataObjectPointerArraySizeType i = 0; i < nb; i++)
  {
    if( this->GetInput(i) == input )
    {
      this->RemoveInput(i);
      return;
    }
  }
}

void mitk::CreateDistanceImageFromSurfaceFilter::Reset()
{
  for (unsigned int i = 0; i < this->GetNumberOfIndexedInputs(); i++)
  {
    this->PopBackInput();
  }
  this->SetNumberOfIndexedInputs(0);
  this->SetNumberOfIndexedOutputs(1);

  mitk::Image::Pointer output = mitk::Image::New();
  this->SetNthOutput(0, output.GetPointer());
}

void mitk::CreateDistanceImageFromSurfaceFilter::SetUseProgressBar(bool status)
{
  this->m_UseProgressBar = status;
}

void mitk::CreateDistanceImageFromSurfaceFilter::SetProgressStepSize(unsigned int stepSize)
{
  this->m_ProgressStepSize = stepSize;
}

void mitk::CreateDistanceImageFromSurfaceFilter::SetReferenceImage( itk::ImageBase<3>::Pointer referenceImage )
{
 m_ReferenceImage = referenceImage;
}

void mitk::CreateDistanceImageFromSurfaceFilter::DetermineBounds( DistanceImageType::PointType &minPointInWorldCoordinates,
                                                                  DistanceImageType::PointType &maxPointInWorldCoordinates,
                                                                  DistanceImageType::IndexType &minPointInIndexCoordinates,
                                                                  DistanceImageType::IndexType &maxPointInIndexCoordinates )
{
  PointType firstCenter = m_Centers.at(0);
  DistanceImageType::PointType tmpPoint;
  tmpPoint[0] = firstCenter[0];
  tmpPoint[1] = firstCenter[1];
  tmpPoint[2] = firstCenter[2];

  // transform the first point from world-coordinates to index-coordinates
  DistanceImageType::IndexType tmpIndex;
  m_ReferenceImage->TransformPhysicalPointToIndex( tmpPoint, tmpIndex );

  // initialize the variables with this first point
  int xmin = tmpIndex[0];
  int ymin = tmpIndex[1];
  int zmin = tmpIndex[2];
  int xmax = tmpIndex[0];
  int ymax = tmpIndex[1];
  int zmax = tmpIndex[2];

  // iterate over the rest of the points
  CenterList::iterator centerIter = m_Centers.begin();
  for ( ++centerIter; centerIter!=m_Centers.end(); centerIter++)
  {
    tmpPoint[0] = (*centerIter)[0];
    tmpPoint[1] = (*centerIter)[1];
    tmpPoint[2] = (*centerIter)[2];

    // transform each point from world-coordinates to index-coordinates
    m_ReferenceImage->TransformPhysicalPointToIndex( tmpPoint, tmpIndex );

    // and set the variables accordingly to find the minimum
    // and maximum in all directions in index-coordinates
    if (xmin > tmpIndex[0])
    {
      xmin = tmpIndex[0];
    }
    if (ymin > tmpIndex[1])
    {
      ymin = tmpIndex[1];
    }
    if (zmin > tmpIndex[2])
    {
      zmin = tmpIndex[2];
    }
    if (xmax < tmpIndex[0])
    {
      xmax = tmpIndex[0];
    }
    if (ymax < tmpIndex[1])
    {
      ymax = tmpIndex[1];
    }
    if (zmax < tmpIndex[2])
    {
      zmax = tmpIndex[2];
    }
  }

  // put the found coordinates into Index-Points
  minPointInIndexCoordinates[0] = xmin;
  minPointInIndexCoordinates[1] = ymin;
  minPointInIndexCoordinates[2] = zmin;

  maxPointInIndexCoordinates[0] = xmax;
  maxPointInIndexCoordinates[1] = ymax;
  maxPointInIndexCoordinates[2] = zmax;

  // and transform them into world-coordinates
  m_ReferenceImage->TransformIndexToPhysicalPoint( minPointInIndexCoordinates, minPointInWorldCoordinates );
  m_ReferenceImage->TransformIndexToPhysicalPoint( maxPointInIndexCoordinates, maxPointInWorldCoordinates );
}

