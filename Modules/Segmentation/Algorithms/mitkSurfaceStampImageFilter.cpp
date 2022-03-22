/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSurfaceStampImageFilter.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageWriteAccessor.h"
#include "mitkTimeHelper.h"
#include <mitkImageCast.h>

#include <vtkCell.h>
#include <vtkLinearTransform.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

#include <itkTriangleMeshToBinaryImageFilter.h>

mitk::SurfaceStampImageFilter::SurfaceStampImageFilter()
  : m_MakeOutputBinary(false), m_OverwriteBackground(false), m_BackgroundValue(0.0), m_ForegroundValue(1.0)
{
}

mitk::SurfaceStampImageFilter::~SurfaceStampImageFilter()
{
}

void mitk::SurfaceStampImageFilter::GenerateInputRequestedRegion()
{
  mitk::Image *outputImage = this->GetOutput();
  if ((outputImage->IsInitialized() == false))
    return;

  GenerateTimeInInputRegion(outputImage, this->GetInput());
}

void mitk::SurfaceStampImageFilter::GenerateOutputInformation()
{
  mitk::Image::ConstPointer inputImage = this->GetInput();
  mitk::Image::Pointer outputImage = this->GetOutput();

  itkDebugMacro(<< "GenerateOutputInformation()");

  if (inputImage.IsNull() || (inputImage->IsInitialized() == false) || (inputImage->GetTimeGeometry() == nullptr))
    return;

  if (m_MakeOutputBinary)
    outputImage->Initialize(mitk::MakeScalarPixelType<unsigned char>(), *inputImage->GetTimeGeometry());
  else
    outputImage->Initialize(inputImage->GetPixelType(), *inputImage->GetTimeGeometry());

  outputImage->SetPropertyList(inputImage->GetPropertyList()->Clone());
}

void mitk::SurfaceStampImageFilter::SetSurface(mitk::Surface *surface)
{
  m_Surface = surface;
}

void mitk::SurfaceStampImageFilter::GenerateData()
{
  mitk::Image::ConstPointer inputImage = this->GetInput();

  if (inputImage.IsNull())
    return;

  mitk::Image::Pointer outputImage = this->GetOutput();
  if (outputImage->IsInitialized() == false)
    return;

  if (m_Surface.IsNull())
    return;

  mitk::Image::RegionType outputRegion = outputImage->GetRequestedRegion();

  int tstart = outputRegion.GetIndex(3);
  int tmax = tstart + outputRegion.GetSize(3);

  if (tmax > 0)
  {
    int t;
    for (t = tstart; t < tmax; ++t)
    {
      this->SurfaceStamp(t);
    }
  }
  else
  {
    this->SurfaceStamp(0);
  }
}

void mitk::SurfaceStampImageFilter::SurfaceStamp(int time)
{
  mitk::Image::Pointer inputImage = this->GetInput();

  const mitk::TimeGeometry *surfaceTimeGeometry = GetInput()->GetTimeGeometry();
  const mitk::TimeGeometry *imageTimeGeometry = inputImage->GetTimeGeometry();

  // Convert time step from image time-frame to surface time-frame
  mitk::TimePointType matchingTimePoint = imageTimeGeometry->TimeStepToTimePoint(time);
  mitk::TimeStepType surfaceTimeStep = surfaceTimeGeometry->TimePointToTimeStep(matchingTimePoint);

  vtkPolyData *polydata = m_Surface->GetVtkPolyData(surfaceTimeStep);
  if (!polydata)
    mitkThrow() << "Polydata is null.";

  vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  transformFilter->SetInputData(polydata);
  //  transformFilter->ReleaseDataFlagOn();

  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  BaseGeometry::Pointer geometry = surfaceTimeGeometry->GetGeometryForTimeStep(surfaceTimeStep);

  transform->PostMultiply();
  transform->Concatenate(geometry->GetVtkTransform()->GetMatrix());
  // take image geometry into account. vtk-Image information will be changed to unit spacing and zero origin below.
  BaseGeometry::Pointer imageGeometry = imageTimeGeometry->GetGeometryForTimeStep(time);

  transform->Concatenate(imageGeometry->GetVtkTransform()->GetLinearInverse());
  transformFilter->SetTransform(transform);
  transformFilter->Update();

  polydata = transformFilter->GetOutput();

  if (!polydata || !polydata->GetNumberOfPoints())
    mitkThrow() << "Polydata retrieved from transformation is null or has no points.";

  MeshType::Pointer mesh = MeshType::New();
  mesh->SetCellsAllocationMethod(itk::MeshEnums::MeshClassCellsAllocationMethod::CellsAllocatedDynamicallyCellByCell);
  unsigned int numberOfPoints = polydata->GetNumberOfPoints();
  mesh->GetPoints()->Reserve(numberOfPoints);

  vtkPoints *points = polydata->GetPoints();

  MeshType::PointType point;
  for (unsigned int i = 0; i < numberOfPoints; i++)
  {
    double *aux = points->GetPoint(i);
    point[0] = aux[0];
    point[1] = aux[1];
    point[2] = aux[2];
    mesh->SetPoint(i, point);
  }

  // Load the polygons into the itk::Mesh
  typedef MeshType::CellAutoPointer CellAutoPointerType;
  typedef MeshType::CellType CellType;
  typedef itk::TriangleCell<CellType> TriangleCellType;
  typedef MeshType::PointIdentifier PointIdentifierType;
  typedef MeshType::CellIdentifier CellIdentifierType;

  // Read the number of polygons
  CellIdentifierType numberOfPolygons = 0;
  numberOfPolygons = polydata->GetNumberOfPolys();

  PointIdentifierType numberOfCellPoints = 3;
  CellIdentifierType i = 0;

  for (i = 0; i < numberOfPolygons; i++)
  {
    vtkIdList *cellIds;
    vtkCell *vcell = polydata->GetCell(i);
    cellIds = vcell->GetPointIds();

    CellAutoPointerType cell;
    auto *triangleCell = new TriangleCellType;
    PointIdentifierType k;
    for (k = 0; k < numberOfCellPoints; k++)
    {
      triangleCell->SetPointId(k, cellIds->GetId(k));
    }

    cell.TakeOwnership(triangleCell);
    mesh->SetCell(i, cell);
  }

  if (!mesh->GetNumberOfPoints())
    mitkThrow() << "Generated itk mesh is empty.";

  if (m_MakeOutputBinary)
  {
    this->SurfaceStampBinaryOutputProcessing(mesh);
  }
  else
  {
    AccessFixedDimensionByItk_1(inputImage, SurfaceStampProcessing, 3, mesh);
  }
}

void mitk::SurfaceStampImageFilter::SurfaceStampBinaryOutputProcessing(MeshType *mesh)
{
  auto *inputImage = this->GetInput();

  mitk::Image::Pointer outputImage = this->GetOutput();

  typedef itk::Image<unsigned char, 3> BinaryImageType;
  BinaryImageType::Pointer itkInputImage;
  mitk::CastToItkImage(inputImage, itkInputImage);

  typedef itk::TriangleMeshToBinaryImageFilter<MeshType, BinaryImageType> FilterType;

  FilterType::Pointer filter = FilterType::New();
  filter->SetInput(mesh);
  filter->SetInfoImage(itkInputImage);
  filter->SetInsideValue(1);
  filter->SetOutsideValue(0);
  filter->Update();

  BinaryImageType::Pointer resultImage = filter->GetOutput();
  resultImage->DisconnectPipeline();

  mitk::CastToMitkImage(resultImage, outputImage);
}

template <typename TPixel>
void mitk::SurfaceStampImageFilter::SurfaceStampProcessing(itk::Image<TPixel, 3> *input, MeshType *mesh)
{
  typedef itk::Image<TPixel, 3> ImageType;
  typedef itk::Image<unsigned char, 3> BinaryImageType;

  typedef itk::TriangleMeshToBinaryImageFilter<mitk::SurfaceStampImageFilter::MeshType, BinaryImageType> FilterType;

  BinaryImageType::Pointer binaryInput = BinaryImageType::New();
  binaryInput->SetSpacing(input->GetSpacing());
  binaryInput->SetOrigin(input->GetOrigin());
  binaryInput->SetDirection(input->GetDirection());
  binaryInput->SetRegions(input->GetLargestPossibleRegion());
  binaryInput->Allocate();
  binaryInput->FillBuffer(0);

  FilterType::Pointer filter = FilterType::New();
  filter->SetInput(mesh);
  filter->SetInfoImage(binaryInput);
  filter->SetInsideValue(1);
  filter->SetOutsideValue(0);
  filter->Update();

  BinaryImageType::Pointer resultImage = filter->GetOutput();
  resultImage->DisconnectPipeline();

  mitk::Image::Pointer outputImage = this->GetOutput();
  typename ImageType::Pointer itkOutputImage;
  mitk::CastToItkImage(outputImage, itkOutputImage);

  typedef itk::ImageRegionConstIterator<BinaryImageType> BinaryIteratorType;
  typedef itk::ImageRegionConstIterator<ImageType> InputIteratorType;
  typedef itk::ImageRegionIterator<ImageType> OutputIteratorType;

  BinaryIteratorType sourceIter(resultImage, resultImage->GetLargestPossibleRegion());
  sourceIter.GoToBegin();

  InputIteratorType inputIter(input, input->GetLargestPossibleRegion());
  inputIter.GoToBegin();

  OutputIteratorType outputIter(itkOutputImage, itkOutputImage->GetLargestPossibleRegion());
  outputIter.GoToBegin();

  typename ImageType::PixelType inputValue;
  unsigned char sourceValue;

  auto fgValue = static_cast<typename ImageType::PixelType>(m_ForegroundValue);
  auto bgValue = static_cast<typename ImageType::PixelType>(m_BackgroundValue);

  while (!sourceIter.IsAtEnd())
  {
    sourceValue = static_cast<unsigned char>(sourceIter.Get());
    inputValue = static_cast<typename ImageType::PixelType>(inputIter.Get());

    if (sourceValue != 0)
      outputIter.Set(fgValue);
    else if (m_OverwriteBackground)
      outputIter.Set(bgValue);
    else
      outputIter.Set(inputValue);

    ++sourceIter;
    ++inputIter;
    ++outputIter;
  }
}
