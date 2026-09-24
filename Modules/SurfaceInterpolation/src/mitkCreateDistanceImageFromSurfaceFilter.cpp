/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCreateDistanceImageFromSurfaceFilter.h>

#include <mitkImageCast.h>
#include <mitkProgressTask.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkMultiThreaderBase.h>

#include <cmath>
#include <queue>
#include <vector>

namespace
{
  /** The interpolated distance function: the distances to all centers, weighted by the solved weights and summed up. */
  class DistanceFunction
  {
  public:
    DistanceFunction(const mitk::CreateDistanceImageFromSurfaceFilter::CenterList& centers, const Eigen::VectorXd& weights)
    {
      m_X.reserve(centers.size());
      m_Y.reserve(centers.size());
      m_Z.reserve(centers.size());
      m_Weights.reserve(centers.size());

      for (std::size_t i = 0; i < centers.size(); ++i)
      {
        m_X.push_back(centers[i][0]);
        m_Y.push_back(centers[i][1]);
        m_Z.push_back(centers[i][2]);
        m_Weights.push_back(weights[static_cast<Eigen::Index>(i)]);
      }
    }

    double operator()(const itk::Point<double, 3>& point) const
    {
      double distance = 0.0;

      for (std::size_t i = 0; i < m_Weights.size(); ++i)
      {
        const double dx = point[0] - m_X[i];
        const double dy = point[1] - m_Y[i];
        const double dz = point[2] - m_Z[i];
        distance += std::sqrt(dx * dx + dy * dy + dz * dz) * m_Weights[i];
      }

      return distance;
    }

  private:
    std::vector<double> m_X;
    std::vector<double> m_Y;
    std::vector<double> m_Z;
    std::vector<double> m_Weights;
  };
}

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
  m_DistanceImageITK->SetSpacing(itk::Vector<double, 3>(m_DistanceImageSpacing));
  m_DistanceImageITK->Allocate();

  // First of all the image is initialized with the value 10*m_DistanceImageSpacing for each pixel
  m_DistanceImageDefaultBufferValue = 10 * m_DistanceImageSpacing;
  m_DistanceImageITK->FillBuffer(m_DistanceImageDefaultBufferValue);

  // Now we move the origin of the distanceImage 2 index-Coordinates
  // in all directions
  auto originAsIndex = m_DistanceImageITK->TransformPhysicalPointToIndex(originAsWorld);
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
  this->m_ProgressTask = nullptr;

  mitk::Image::Pointer output = mitk::Image::New();
  this->SetNthOutput(0, output.GetPointer());
}

mitk::CreateDistanceImageFromSurfaceFilter::~CreateDistanceImageFromSurfaceFilter()
{
}

void mitk::CreateDistanceImageFromSurfaceFilter::GenerateData()
{
  if (nullptr != m_ProgressTask)
    m_ProgressTask->AddStepsToDo(5);

  this->PreprocessContourPoints();
  this->CreateEmptyDistanceImage();

  // First of all we have to build the equation-system from the existing contour-edge-points
  this->CreateSolutionMatrixAndFunctionValues();

  if (nullptr != m_ProgressTask)
    m_ProgressTask->Progress(1);

  m_Weights = m_SolutionMatrix.partialPivLu().solve(m_FunctionValues);

  if (nullptr != m_ProgressTask)
    m_ProgressTask->Progress(2);

  // The last step is to create the distance map with the interpolated distance function
  this->FillDistanceImage();

  if (nullptr != m_ProgressTask)
    m_ProgressTask->Progress(2);

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

  // First of all we have to extract the normals and the surface points.
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

    const vtkIdType *cell(nullptr);
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
      // Calculate the RBF value. Currently using Phi(r) = r with r is the euclidean distance between two points
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
  * Only the pixels close to the contours get their distance: a region grows from the pixel of the
  * first contour point, and a pixel next to the region (6-neighborhood) joins it if its distance is
  * within twice the spacing. The distances of all pixels are computed up front in parallel, which is
  * cheaper than computing the ones the growing region reaches one after another.
  */

  typedef itk::ImageRegionIteratorWithIndex<DistanceImageType> ImageIterator;

  const DistanceFunction distanceFunction(m_Centers, m_Weights);

  const auto region = m_DistanceImageITK->GetLargestPossibleRegion();
  const std::size_t sizeX = region.GetSize(0);
  const std::size_t sizeY = region.GetSize(1);
  const std::size_t sizeZ = region.GetSize(2);
  const std::size_t sliceSize = sizeX * sizeY;

  std::vector<double> distances(sliceSize * sizeZ);

  itk::MultiThreaderBase::New()->ParallelizeImageRegion<3>(region, [&](const DistanceImageType::RegionType& chunk)
    {
      DistanceImageType::PointType point;

      for (itk::ImageRegionConstIteratorWithIndex<DistanceImageType> it(m_DistanceImageITK, chunk); !it.IsAtEnd(); ++it)
      {
        m_DistanceImageITK->TransformIndexToPhysicalPoint(it.GetIndex(), point);
        distances[m_DistanceImageITK->ComputeOffset(it.GetIndex())] = distanceFunction(point);
      }
    }, nullptr);

  auto* pixels = m_DistanceImageITK->GetBufferPointer();

  DistanceImageType::PointType seedPoint;
  seedPoint[0] = m_Centers.at(0)[0];
  seedPoint[1] = m_Centers.at(0)[1];
  seedPoint[2] = m_Centers.at(0)[2];

  const auto seedIndex = m_DistanceImageITK->TransformPhysicalPointToIndex(seedPoint);

  assert(
    m_DistanceImageITK->GetLargestPossibleRegion().IsInside(seedIndex)); // we are quite certain this should hold

  const auto seedOffset = static_cast<std::size_t>(m_DistanceImageITK->ComputeOffset(seedIndex));
  pixels[seedOffset] = distanceFunction(seedPoint);

  std::vector<bool> reached(sliceSize * sizeZ, false);
  reached[seedOffset] = true;

  std::queue<std::size_t> pending;
  pending.push(seedOffset);

  const auto visit = [&](std::size_t offset)
    {
      if (!reached[offset] && std::fabs(distances[offset]) <= m_DistanceImageSpacing * 2)
      {
        reached[offset] = true;
        pixels[offset] = distances[offset];
        pending.push(offset);
      }
    };

  while (!pending.empty())
  {
    const auto offset = pending.front();
    pending.pop();

    const auto x = offset % sizeX;
    const auto y = (offset / sizeX) % sizeY;
    const auto z = offset / sliceSize;

    if (x > 0) visit(offset - 1);
    if (x + 1 < sizeX) visit(offset + 1);
    if (y > 0) visit(offset - sizeX);
    if (y + 1 < sizeY) visit(offset + sizeX);
    if (z > 0) visit(offset - sliceSize);
    if (z + 1 < sizeZ) visit(offset + sliceSize);
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

void mitk::CreateDistanceImageFromSurfaceFilter::GenerateOutputInformation()
{
}

void mitk::CreateDistanceImageFromSurfaceFilter::PrintEquationSystem()
{
  std::stringstream out;
  out << "Number of rows: " << m_SolutionMatrix.rows() << " ****** Number of columns: " << m_SolutionMatrix.cols()
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

void mitk::CreateDistanceImageFromSurfaceFilter::SetProgressTask(ProgressTask* task)
{
  m_ProgressTask = task;
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
  auto tmpIndex = m_ReferenceImage->TransformPhysicalPointToContinuousIndex<DistanceImageType::PointType::ValueType>(tmpPoint);

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
    tmpIndex = m_ReferenceImage->TransformPhysicalPointToContinuousIndex<DistanceImageType::PointType::ValueType>(tmpPoint);

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
