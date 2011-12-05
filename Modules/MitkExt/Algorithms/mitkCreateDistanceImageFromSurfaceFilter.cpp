/*=========================================================================
Program: Medical Imaging & Interaction Toolkit
Module: $RCSfile$
Language: C++
Date: $Date: $
Version: $Revision: $
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the above copyright notices for more information.
=========================================================================*/

#include "mitkCreateDistanceImageFromSurfaceFilter.h"


mitk::CreateDistanceImageFromSurfaceFilter::CreateDistanceImageFromSurfaceFilter()
{
  m_DistanceImageVolume = 50000;
}


mitk::CreateDistanceImageFromSurfaceFilter::~CreateDistanceImageFromSurfaceFilter()
{
}

void mitk::CreateDistanceImageFromSurfaceFilter::GenerateData()
{
  //First of all we have to build the equation-system from the existing contour-edge-points
  this->CreateSolutionMatrixAndFunctionValues();

  //Then we solve the equation-system via QR - decomposition. The interpolation weights are obtained in that way
  vnl_qr<double> solver (m_SolutionMatrix);
  m_Weights = solver.solve(m_FunctionValues);

  //The last step is to create the distance map with the interpolated distance function
  this->CreateDistanceImage();
  m_Centers.clear();
  m_FunctionValues.clear();
  m_Normals.clear();
  m_Weights.clear();
  m_SolutionMatrix.clear();
}

void mitk::CreateDistanceImageFromSurfaceFilter::CreateSolutionMatrixAndFunctionValues()
{
  unsigned int numberOfInputs = this->GetNumberOfInputs();

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
      for ( unsigned int j = 0; j < cellSize; j++ )
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

  m_FunctionValues.set_size(numberOfCenters*3);
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

    m_FunctionValues.put(numberOfCenters+i, -1);
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

    m_FunctionValues.put(numberOfCenters*2+i, 1);
  }

  //Now we have created all centers and all function values. Next step is to create the solution matrix
  numberOfCenters = m_Centers.size();
  m_SolutionMatrix.set_size(numberOfCenters, numberOfCenters);

  m_Weights.set_size(numberOfCenters);

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
  typedef itk::Image<double, 3> DistanceImageType;
  typedef itk::ImageRegionIteratorWithIndex<DistanceImageType> ImageIterator;
  typedef itk::NeighborhoodIterator<DistanceImageType> NeighborhoodImageIterator;

  DistanceImageType::Pointer distanceImg = DistanceImageType::New();

  //Determin the bounding box of the delineated contours
  double xmin = m_Centers.at(0)[0];
  double ymin = m_Centers.at(0)[1];
  double zmin = m_Centers.at(0)[2];
  double xmax = m_Centers.at(0)[0];
  double ymax = m_Centers.at(0)[1];
  double zmax = m_Centers.at(0)[2];

  for (unsigned int i = 1; i < m_Centers.size(); i++)
  {
    if (xmin > m_Centers.at(i)[0])
    {
      xmin = m_Centers.at(i)[0];
    }
    if (ymin > m_Centers.at(i)[1])
    {
      ymin = m_Centers.at(i)[1];
    }
    if (zmin > m_Centers.at(i)[2])
    {
      zmin = m_Centers.at(i)[2];
    }
    if (xmax < m_Centers.at(i)[0])
    {
      xmax = m_Centers.at(i)[0];
    }
    if (ymax < m_Centers.at(i)[1])
    {
      ymax = m_Centers.at(i)[1];
    }
    if (zmax < m_Centers.at(i)[2])
    {
      zmax = m_Centers.at(i)[2];
    }
  }

  Vector3D extentMM;
  extentMM[0] = xmax - xmin + 2;
  extentMM[1] = ymax - ymin + 2;
  extentMM[2] = zmax - zmin + 2;

  //Shifting the distance image's offest to achieve an exact distance calculation
  xmin = xmin - 2;
  ymin = ymin - 2;
  zmin = zmin - 2;

  /*
    Now create an empty distance image. The create image will always have the same size, independent from
    the original image (e.g. always consists of 500000 pixels) and will have an isotropic spacing.
    The spacing is calculated like the following:
    The image's volume = 500000 Pixels = extentX*spacing*extentY*spacing*extentZ*spacing
    So the spacing is: spacing = ( 500000 / extentX*extentY*extentZ )^(1/3)
  */

  double basis = (extentMM[0]*extentMM[1]*extentMM[2]) / m_DistanceImageVolume;
  double exponent = 1.0/3.0;
  double distImgSpacing = pow(basis, exponent);
  int tempSpacing = (distImgSpacing+0.05)*10;
  m_DistanceImageSpacing = (double)tempSpacing/10.0;

  unsigned int numberOfXPixel = extentMM[0] / m_DistanceImageSpacing;
  unsigned int numberOfYPixel = extentMM[1] / m_DistanceImageSpacing;
  unsigned int numberOfZPixel = extentMM[2] / m_DistanceImageSpacing;

  DistanceImageType::SizeType size;

  //Increase the distance image's size a little bit to achieve an exact distance calculation
  size[0] = numberOfXPixel + 5;
  size[1] = numberOfYPixel + 5;
  size[2] = numberOfZPixel + 5;

  DistanceImageType::IndexType start;
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;

  DistanceImageType::RegionType lpRegion;

  lpRegion.SetSize(size);
  lpRegion.SetIndex(start);

  distanceImg->SetRegions( lpRegion );
  distanceImg->SetSpacing( m_DistanceImageSpacing );
  distanceImg->Allocate();

  //First of all the image is initialized with the value 10 for each pixel
  distanceImg->FillBuffer(10);

  /*
    Now we must caculate the distance for each pixel. But instead of calculating the distance value
    for all of the image's pixels we proceed similar to the region growing algorithm:

    1. Take the first pixel from the narrowband_point_list and calculate the distance for each neighbor (6er)
    2. If the current index's distance value is below a certain threshold push it into the list
    3. Next iteration take the next index from the list and start with 1. again

    This is done until the narrowband_point_list is empty.
  */
  std::queue<DistanceImageType::IndexType> narrowbandPoints;
  PointType currentPoint = m_Centers.at(0);
  double distance = this->CalculateDistanceValue(currentPoint);

  DistanceImageType::IndexType currentIndex;
  currentIndex[0] = ( currentPoint[0]-xmin ) / m_DistanceImageSpacing;
  currentIndex[1] = ( currentPoint[1]-ymin ) / m_DistanceImageSpacing;
  currentIndex[2] = ( currentPoint[2]-zmin ) / m_DistanceImageSpacing;

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

    for (int i = 0; i < 6; i++)
    {
      nIt.GetPixel(relativeNbIdx[i], isInBounds);
      if( isInBounds && nIt.GetPixel(relativeNbIdx[i]) == 10)
      {
        currentIndex = nIt.GetIndex(relativeNbIdx[i]);

        currentPoint[0] = currentIndex[0]*m_DistanceImageSpacing + xmin;
        currentPoint[1] = currentIndex[1]*m_DistanceImageSpacing + ymin;
        currentPoint[2] = currentIndex[2]*m_DistanceImageSpacing + zmin;

        distance = this->CalculateDistanceValue(currentPoint);
        if ( abs(distance) <= m_DistanceImageSpacing*2 )
        {
          nIt.SetPixel(relativeNbIdx[i], distance);
          narrowbandPoints.push(currentIndex);
        }
      }
    }
  }

  ImageIterator imgRegionIterator (distanceImg, distanceImg->GetLargestPossibleRegion());
  imgRegionIterator.GoToBegin();

  double prevPixelVal = 1;

  //Set every pixel inside the surface to -10 except the edge point (so that the received surface is closed)
  while (!imgRegionIterator.IsAtEnd()) {

    if ( imgRegionIterator.Get() == 10 && prevPixelVal < 0 )
    {

      while (imgRegionIterator.Get() == 10)
      {
        if (imgRegionIterator.GetIndex()[0] == size[0]-1 || imgRegionIterator.GetIndex()[1] == size[1]-1 || imgRegionIterator.GetIndex()[2] == size[2]-1 
            || imgRegionIterator.GetIndex()[0] == 0 || imgRegionIterator.GetIndex()[1] == 0 || imgRegionIterator.GetIndex()[2] == 0 )
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
    else if (imgRegionIterator.GetIndex()[0] == size[0]-1 || imgRegionIterator.GetIndex()[1] == size[1]-1 || imgRegionIterator.GetIndex()[2] == size[2]-1 
            || imgRegionIterator.GetIndex()[0] == 0 || imgRegionIterator.GetIndex()[1] == 0 || imgRegionIterator.GetIndex()[2] == 0)
    {
      imgRegionIterator.Set(10);
      prevPixelVal = 10;
      ++imgRegionIterator;
    }
    else {
        prevPixelVal = imgRegionIterator.Get();
        ++imgRegionIterator;
    }

  }

  Image::Pointer resultImage = this->GetOutput();

  Point3D origin;
  origin[0] = xmin;
  origin[1] = ymin;
  origin[2] = zmin;

  CastToMitkImage(distanceImg, resultImage);
  resultImage->GetGeometry()->SetOrigin(origin);
  resultImage->SetOrigin(origin);
}


double mitk::CreateDistanceImageFromSurfaceFilter::CalculateDistanceValue(PointType p)
{
  double distanceValue (0);
  PointType p1;
  PointType p2;
  double norm;

  for (unsigned int i = 0; i < m_Centers.size(); i++)
  {
    p1 = m_Centers.at(i);
    p2 = p-p1;
    norm = p2.two_norm();
    distanceValue = distanceValue + norm*m_Weights.get(i);
  }
  return distanceValue;
}

void mitk::CreateDistanceImageFromSurfaceFilter::GenerateOutputInformation()
{
}

void mitk::CreateDistanceImageFromSurfaceFilter::PrintEquationSystem()
{
  std::ofstream esfile;
  esfile.open("C:/Users/fetzer/Desktop/equationSystem/es.txt");
  esfile<<"Nummber of rows: "<<m_SolutionMatrix.rows()<<" ****** Number of columns: "<<m_SolutionMatrix.columns()<<endl;
  esfile<<"[ ";
  for (unsigned int i = 0; i < m_SolutionMatrix.rows(); i++)
  {
    for (unsigned int j = 0; j < m_SolutionMatrix.columns(); j++)
    {
      esfile<<m_SolutionMatrix(i,j)<<"   ";
    }
    esfile<<";"<<endl;
  }
  esfile<<" ]";
  esfile.close();

  std::ofstream centersFile;
  centersFile.open("C:/Users/fetzer/Desktop/equationSystem/centers.txt");
  for (unsigned int i = 0; i < m_Centers.size(); i++)
  {
    centersFile<<m_Centers.at(i)<<";"<<endl;
  }
  centersFile.close();

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
    if (this->GetNumberOfInputs() < 1)
        return NULL;

    return static_cast<const mitk::Surface*>(this->ProcessObject::GetInput(0));
}


const mitk::Surface* mitk::CreateDistanceImageFromSurfaceFilter::GetInput( unsigned int idx)
{
    if (this->GetNumberOfInputs() < 1)
        return NULL;

    return static_cast<const mitk::Surface*>(this->ProcessObject::GetInput(idx));
}

void mitk::CreateDistanceImageFromSurfaceFilter::RemoveInputs(mitk::Surface* input)
{
    this->RemoveInput(input);
}

void mitk::CreateDistanceImageFromSurfaceFilter::Reset()
{
  for (unsigned int i = 0; i < this->GetNumberOfInputs(); i++)
  {
    this->PopBackInput();
  }
  this->SetNumberOfInputs(0);
  this->SetNumberOfOutputs(1);
}
