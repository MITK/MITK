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

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"

#include "itkImageRegionIteratorWithIndex.h"
#include "itkNeighborhoodIterator.h"

#include <queue>

void mitk::CreateDistanceImageFromSurfaceFilter::CreateEmptyDistanceImage()
{
  // Determine the bounds of the input points in index- and world-coordinates
  DistanceImageType::PointType minPointInWorldCoordinates, maxPointInWorldCoordinates;
  DistanceImageType::IndexType minPointInIndexCoordinates, maxPointInIndexCoordinates;

  DetermineBounds(
    minPointInWorldCoordinates, maxPointInWorldCoordinates, minPointInIndexCoordinates, maxPointInIndexCoordinates);

  // Calculate the extent of the region that contains all given points in MM.
  // To do this, we take the difference between the maximal and minimal
  // index-coordinates (must not be less than 1) and multiply it with the
  // spacing of the reference-image.
  Vector3D extentMM;
  for (unsigned int dim = 0; dim < 3; ++dim)
  {
    extentMM[dim] = (std::abs(maxPointInIndexCoordinates[dim] - minPointInIndexCoordinates[dim])) *
                    m_ReferenceImage->GetSpacing()[dim];
  }

  /*
  * Now create an empty distance image. The created image will always have the same number of pixels, independent from
  * the original image (e.g. always consists of 500000 pixels) and will have an isotropic spacing.
  * The spacing is calculated like the following:
  * The image's volume = 500000 Pixels = extentX*spacing*extentY*spacing*extentZ*spacing
  * So the spacing is: spacing = ( extentX*extentY*extentZ / 500000 )^(1/3)
  */
  double basis = (extentMM[0] * extentMM[1] * extentMM[2]) / m_DistanceImageVolume;
  double exponent = 1.0 / 3.0;
  m_DistanceImageSpacing = pow(basis, exponent);

  // calculate the number of pixels of the distance image for each direction
  unsigned int numberOfXPixel = extentMM[0] / m_DistanceImageSpacing;
  unsigned int numberOfYPixel = extentMM[1] / m_DistanceImageSpacing;
  unsigned int numberOfZPixel = extentMM[2] / m_DistanceImageSpacing;

  // We increase the sizeOfRegion by 4 as we decrease the origin by 2 later.
  // This expansion of the region is necessary to achieve a complete
  // interpolation.
  DistanceImageType::SizeType sizeOfRegion;
  sizeOfRegion[0] = numberOfXPixel + 8;
  sizeOfRegion[1] = numberOfYPixel + 8;
  sizeOfRegion[2] = numberOfZPixel + 8;

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
  m_DistanceImageITK = DistanceImageType::New();
  m_DistanceImageITK->SetOrigin(originAsWorld);
  m_DistanceImageITK->SetDirection(m_ReferenceImage->GetDirection());
  m_DistanceImageITK->SetRegions(lpRegion);
  m_DistanceImageITK->SetSpacing(m_DistanceImageSpacing);
  m_DistanceImageITK->Allocate();

  // First of all the image is initialized with the value 10*m_DistanceImageSpacing for each pixel
  m_DistanceImageDefaultBufferValue = 10 * m_DistanceImageSpacing;
  m_DistanceImageITK->FillBuffer(m_DistanceImageDefaultBufferValue);

  // Now we move the origin of the distanceImage 2 index-Coordinates
  // in all directions
  DistanceImageType::IndexType originAsIndex;
  m_DistanceImageITK->TransformPhysicalPointToIndex(originAsWorld, originAsIndex);
  originAsIndex[0] -= 2;
  originAsIndex[1] -= 2;
  originAsIndex[2] -= 2;
  m_DistanceImageITK->TransformIndexToPhysicalPoint(originAsIndex, originAsWorld);
  m_DistanceImageITK->SetOrigin(originAsWorld);
}

mitk::CreateDistanceImageFromSurfaceFilter::CreateDistanceImageFromSurfaceFilter()
  : m_DistanceImageSpacing(0.0), m_DistanceImageDefaultBufferValue(0.0)
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
  this->PreprocessContourPoints();
  this->CreateEmptyDistanceImage();

  // First of all we have to build the equation-system from the existing contour-edge-points
  this->CreateSolutionMatrixAndFunctionValues();

  if (this->m_UseProgressBar)
    mitk::ProgressBar::GetInstance()->Progress(1);

  m_Weights = m_SolutionMatrix.partialPivLu().solve(m_FunctionValues);

  if (this->m_UseProgressBar)
    mitk::ProgressBar::GetInstance()->Progress(2);

  // The last step is to create the distance map with the interpolated distance function
  this->FillDistanceImage();

  if (this->m_UseProgressBar)
    mitk::ProgressBar::GetInstance()->Progress(2);

  m_Centers.clear();
  m_Normals.clear();
}

void mitk::CreateDistanceImageFromSurfaceFilter::PreprocessContourPoints()
{
  unsigned int numberOfInputs = this->GetNumberOfIndexedInputs();

  if (numberOfInputs == 0)
  {
    MITK_ERROR << "mitk::CreateDistanceImageFromSurfaceFilter: No input available. Please set an input!" << std::endl;
    itkExceptionMacro("mitk::CreateDistanceImageFromSurfaceFilter: No input available. Please set an input!");
    return;
  }

  // First of all we have to extract the nomals and the surface points.
  // Duplicated points can be eliminated

  vtkSmartPointer<vtkPolyData> polyData;
  vtkSmartPointer<vtkDoubleArray> currentCellNormals;
  vtkSmartPointer<vtkCellArray> existingPolys;
  vtkSmartPointer<vtkPoints> existingPoints;

  double p[3];
  PointType currentPoint;
  PointType normal;

  for (unsigned int i = 0; i < numberOfInputs; i++)
  {
    auto currentSurface = this->GetInput(i);
    polyData = currentSurface->GetVtkPolyData();

    if (polyData->GetNumberOfPolys() == 0)
    {
      MITK_INFO << "mitk::CreateDistanceImageFromSurfaceFilter: No input-polygons available. Please be sure the input "
                   "surface consists of polygons!"
                << std::endl;
    }

    currentCellNormals = vtkDoubleArray::SafeDownCast(polyData->GetCellData()->GetNormals());

    existingPolys = polyData->GetPolys();

    existingPoints = polyData->GetPoints();

    existingPolys->InitTraversal();

    vtkIdType *cell(nullptr);
    vtkIdType cellSize(0);

    for (existingPolys->InitTraversal(); existingPolys->GetNextCell(cellSize, cell);)
    {
      for (vtkIdType j = 0; j < cellSize; j++)
      {
        existingPoints->GetPoint(cell[j], p);

        currentPoint.copy_in(p);

        int count = std::count(m_Centers.begin(), m_Centers.end(), currentPoint);

        if (count == 0)
        {
          double currentNormal[3];
          currentCellNormals->GetTuple(cell[j], currentNormal);

          normal.copy_in(currentNormal);

          m_Normals.push_back(normal);

          m_Centers.push_back(currentPoint);
        }

      } // end for all points
    }   // end for all cells
  }     // end for all outputs
}

void mitk::CreateDistanceImageFromSurfaceFilter::CreateSolutionMatrixAndFunctionValues()
{
  // For we can now calculate the exact size of the centers we initialize the data structures
  unsigned int numberOfCenters = m_Centers.size();
  m_Centers.reserve(numberOfCenters * 3);

  m_FunctionValues.resize(numberOfCenters * 3);

  m_FunctionValues.fill(0);

  PointType currentPoint;
  PointType normal;

  // Create inner points
  for (unsigned int i = 0; i < numberOfCenters; i++)
  {
    currentPoint = m_Centers.at(i);
    normal = m_Normals.at(i);

    currentPoint[0] = currentPoint[0] - normal[0] * m_DistanceImageSpacing;
    currentPoint[1] = currentPoint[1] - normal[1] * m_DistanceImageSpacing;
    currentPoint[2] = currentPoint[2] - normal[2] * m_DistanceImageSpacing;

    m_Centers.push_back(currentPoint);

    m_FunctionValues[numberOfCenters + i] = -m_DistanceImageSpacing;
  }

  // Create outer points
  for (unsigned int i = 0; i < numberOfCenters; i++)
  {
    currentPoint = m_Centers.at(i);
    normal = m_Normals.at(i);

    currentPoint[0] = currentPoint[0] + normal[0] * m_DistanceImageSpacing;
    currentPoint[1] = currentPoint[1] + normal[1] * m_DistanceImageSpacing;
    currentPoint[2] = currentPoint[2] + normal[2] * m_DistanceImageSpacing;

    m_Centers.push_back(currentPoint);

    m_FunctionValues[numberOfCenters * 2 + i] = m_DistanceImageSpacing;
  }

  // Now we have created all centers and all function values. Next step is to create the solution matrix
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
      // Calculate the RBF value. Currently using Phi(r) = r with r is the euclidian distance between two points
      p1 = m_Centers.at(i);
      p2 = m_Centers.at(j);
      p1 = p1 - p2;
      norm = p1.two_norm();
      m_SolutionMatrix(i, j) = norm;
    }
  }
}

void mitk::CreateDistanceImageFromSurfaceFilter::FillDistanceImage()
{
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

  typedef itk::ImageRegionIteratorWithIndex<DistanceImageType> ImageIterator;
  typedef itk::NeighborhoodIterator<DistanceImageType> NeighborhoodImageIterator;

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
  m_DistanceImageITK->TransformPhysicalPointToIndex(currentPointAsPoint, currentIndex);

  assert(
    m_DistanceImageITK->GetLargestPossibleRegion().IsInside(currentIndex)); // we are quite certain this should hold

  narrowbandPoints.push(currentIndex);
  m_DistanceImageITK->SetPixel(currentIndex, distance);

  NeighborhoodImageIterator::RadiusType radius;
  radius.Fill(1);
  NeighborhoodImageIterator nIt(radius, m_DistanceImageITK, m_DistanceImageITK->GetLargestPossibleRegion());
  unsigned int relativeNbIdx[] = {4, 10, 12, 14, 16, 22};

  bool isInBounds = false;
  while (!narrowbandPoints.empty())
  {
    nIt.SetLocation(narrowbandPoints.front());
    narrowbandPoints.pop();

    unsigned int *relativeNb = &relativeNbIdx[0];
    for (int i = 0; i < 6; i++)
    {
      nIt.GetPixel(*relativeNb, isInBounds);
      if (isInBounds && nIt.GetPixel(*relativeNb) == m_DistanceImageDefaultBufferValue)
      {
        currentIndex = nIt.GetIndex(*relativeNb);

        // Transform the currently checked point from index-coordinates to
        // world-coordinates
        m_DistanceImageITK->TransformIndexToPhysicalPoint(currentIndex, currentPointAsPoint);

        // create a vnl_vector
        currentPoint[0] = currentPointAsPoint[0];
        currentPoint[1] = currentPointAsPoint[1];
        currentPoint[2] = currentPointAsPoint[2];

        // and check the distance
        distance = this->CalculateDistanceValue(currentPoint);
        if (std::fabs(distance) <= m_DistanceImageSpacing * 2)
        {
          nIt.SetPixel(*relativeNb, distance);
          narrowbandPoints.push(currentIndex);
        }
      }
      relativeNb++;
    }
  }

  ImageIterator imgRegionIterator(m_DistanceImageITK, m_DistanceImageITK->GetLargestPossibleRegion());
  imgRegionIterator.GoToBegin();

  double prevPixelVal = 1;

  DistanceImageType::IndexType _size;
  _size.Fill(-1);
  _size += m_DistanceImageITK->GetLargestPossibleRegion().GetSize();

  // Set every pixel inside the surface to -m_DistanceImageDefaultBufferValue except the edge point (so that the
  // received surface is closed)
  while (!imgRegionIterator.IsAtEnd())
  {
    if (imgRegionIterator.Get() == m_DistanceImageDefaultBufferValue && prevPixelVal < 0)
    {
      while (imgRegionIterator.Get() == m_DistanceImageDefaultBufferValue)
      {
        if (imgRegionIterator.GetIndex()[0] == _size[0] || imgRegionIterator.GetIndex()[1] == _size[1] ||
            imgRegionIterator.GetIndex()[2] == _size[2] || imgRegionIterator.GetIndex()[0] == 0U ||
            imgRegionIterator.GetIndex()[1] == 0U || imgRegionIterator.GetIndex()[2] == 0U)
        {
          imgRegionIterator.Set(m_DistanceImageDefaultBufferValue);
          prevPixelVal = m_DistanceImageDefaultBufferValue;
          ++imgRegionIterator;
          break;
        }
        else
        {
          imgRegionIterator.Set((-1) * m_DistanceImageDefaultBufferValue);
          ++imgRegionIterator;
          prevPixelVal = (-1) * m_DistanceImageDefaultBufferValue;
        }
      }
    }
    else if (imgRegionIterator.GetIndex()[0] == _size[0] || imgRegionIterator.GetIndex()[1] == _size[1] ||
             imgRegionIterator.GetIndex()[2] == _size[2] || imgRegionIterator.GetIndex()[0] == 0U ||
             imgRegionIterator.GetIndex()[1] == 0U || imgRegionIterator.GetIndex()[2] == 0U)

    {
      imgRegionIterator.Set(m_DistanceImageDefaultBufferValue);
      prevPixelVal = m_DistanceImageDefaultBufferValue;
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
  CastToMitkImage(m_DistanceImageITK, resultImage);
}

double mitk::CreateDistanceImageFromSurfaceFilter::CalculateDistanceValue(PointType p)
{
  double distanceValue(0);
  PointType p1;
  PointType p2;
  double norm;

  CenterList::iterator centerIter;

  unsigned int count(0);
  for (centerIter = m_Centers.begin(); centerIter != m_Centers.end(); centerIter++)
  {
    p1 = *centerIter;
    p2 = p - p1;
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
  out << "Nummber of rows: " << m_SolutionMatrix.rows() << " ****** Number of columns: " << m_SolutionMatrix.cols()
      << endl;
  out << "[ ";
  for (int i = 0; i < m_SolutionMatrix.rows(); i++)
  {
    for (int j = 0; j < m_SolutionMatrix.cols(); j++)
    {
      out << m_SolutionMatrix(i, j) << "   ";
    }
    out << ";" << endl;
  }
  out << " ]\n\n\n";

  for (unsigned int i = 0; i < m_Centers.size(); i++)
  {
    out << m_Centers.at(i) << ";" << endl;
  }
  std::cout << "Equation system: \n\n\n" << out.str();
}

void mitk::CreateDistanceImageFromSurfaceFilter::SetInput(const mitk::Surface *surface)
{
  this->SetInput(0, surface);
}

void mitk::CreateDistanceImageFromSurfaceFilter::SetInput(unsigned int idx, const mitk::Surface *surface)
{
  if (this->GetInput(idx) != surface)
  {
    this->SetNthInput(idx, const_cast<mitk::Surface *>(surface));
    this->Modified();
  }
}

const mitk::Surface *mitk::CreateDistanceImageFromSurfaceFilter::GetInput()
{
  if (this->GetNumberOfIndexedInputs() < 1)
    return nullptr;

  return static_cast<const mitk::Surface *>(this->ProcessObject::GetInput(0));
}

const mitk::Surface *mitk::CreateDistanceImageFromSurfaceFilter::GetInput(unsigned int idx)
{
  if (this->GetNumberOfIndexedInputs() < 1)
    return nullptr;

  return static_cast<const mitk::Surface *>(this->ProcessObject::GetInput(idx));
}

void mitk::CreateDistanceImageFromSurfaceFilter::RemoveInputs(mitk::Surface *input)
{
  DataObjectPointerArraySizeType nb = this->GetNumberOfIndexedInputs();

  for (DataObjectPointerArraySizeType i = 0; i < nb; i++)
  {
    if (this->GetInput(i) == input)
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

void mitk::CreateDistanceImageFromSurfaceFilter::SetReferenceImage(itk::ImageBase<3>::Pointer referenceImage)
{
  m_ReferenceImage = referenceImage;
}

void mitk::CreateDistanceImageFromSurfaceFilter::DetermineBounds(
  DistanceImageType::PointType &minPointInWorldCoordinates,
  DistanceImageType::PointType &maxPointInWorldCoordinates,
  DistanceImageType::IndexType &minPointInIndexCoordinates,
  DistanceImageType::IndexType &maxPointInIndexCoordinates)
{
  PointType firstCenter = m_Centers.at(0);
  DistanceImageType::PointType tmpPoint;
  tmpPoint[0] = firstCenter[0];
  tmpPoint[1] = firstCenter[1];
  tmpPoint[2] = firstCenter[2];

  // transform the first point from world-coordinates to index-coordinates
  itk::ContinuousIndex<double, 3> tmpIndex;
  m_ReferenceImage->TransformPhysicalPointToContinuousIndex(tmpPoint, tmpIndex);

  // initialize the variables with this first point
  DistanceImageType::IndexValueType xmin = tmpIndex[0];
  DistanceImageType::IndexValueType ymin = tmpIndex[1];
  DistanceImageType::IndexValueType zmin = tmpIndex[2];
  DistanceImageType::IndexValueType xmax = tmpIndex[0];
  DistanceImageType::IndexValueType ymax = tmpIndex[1];
  DistanceImageType::IndexValueType zmax = tmpIndex[2];

  // iterate over the rest of the points
  auto centerIter = m_Centers.begin();
  for (++centerIter; centerIter != m_Centers.end(); centerIter++)
  {
    tmpPoint[0] = (*centerIter)[0];
    tmpPoint[1] = (*centerIter)[1];
    tmpPoint[2] = (*centerIter)[2];

    // transform each point from world-coordinates to index-coordinates
    m_ReferenceImage->TransformPhysicalPointToContinuousIndex(tmpPoint, tmpIndex);

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
  m_ReferenceImage->TransformIndexToPhysicalPoint(minPointInIndexCoordinates, minPointInWorldCoordinates);
  m_ReferenceImage->TransformIndexToPhysicalPoint(maxPointInIndexCoordinates, maxPointInWorldCoordinates);
}
