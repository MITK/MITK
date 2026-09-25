/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkSurfaceToImageFilter.h>
#include <mitkExceptionMacro.h>
#include <mitkImageWriteAccessor.h>
#include <mitkTimeHelper.h>
#include <mitkImageReadAccessor.h>
#include <mitkPixelTypeMultiplex.h>

#include <itkMultiThreaderBase.h>

#include <vtkImageStencilData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>

namespace
{
  template <typename TComponent>
  void FillComponents(const mitk::PixelType&, char* data, std::size_t count, double value)
  {
    const auto clampedValue = std::clamp(value,
      static_cast<double>(std::numeric_limits<TComponent>::lowest()),
      static_cast<double>(std::numeric_limits<TComponent>::max()));

    std::fill_n(reinterpret_cast<TComponent*>(data), count, static_cast<TComponent>(clampedValue));
  }
}

void mitk::ForEachVoxelRunInsideSurface(const Surface* surface, TimeStepType surfaceTimeStep,
  const Image* image, TimeStepType imageTimeStep, double tolerance,
  const std::function<void(std::size_t x, std::size_t y, std::size_t z, std::size_t count)>& visitRun)
{
  if (nullptr == surface)
    mitkThrow() << "Cannot rasterize surface. Passed surface is nullptr.";

  if (nullptr == image)
    mitkThrow() << "Cannot rasterize surface. Passed image is nullptr.";

  if (!image->GetTimeGeometry()->IsValidTimeStep(imageTimeStep))
    mitkThrow() << "Cannot rasterize surface. The image does not have time step " << imageTimeStep << ".";

  auto* polyData = surface->GetVtkPolyData(static_cast<unsigned int>(surfaceTimeStep));

  if (nullptr == polyData || 0 == polyData->GetNumberOfPoints())
    return;

  BaseGeometry::Pointer surfaceGeometry = surface->GetTimeGeometry()->GetGeometryForTimeStep(surfaceTimeStep);

  if (surfaceGeometry.IsNull())
    surfaceGeometry = surface->GetGeometry();

  // Index coordinates put the voxel centers on integers, so a stencil with unit spacing and zero origin
  // samples exactly the voxels of the image.
  auto transform = vtkSmartPointer<vtkTransform>::New();
  transform->PostMultiply();
  transform->Concatenate(surfaceGeometry->GetVtkTransform()->GetMatrix());
  transform->Concatenate(image->GetGeometry(static_cast<int>(imageTimeStep))->GetVtkTransform()->GetLinearInverse());

  auto transformFilter = vtkSmartPointer<vtkTransformFilter>::New();
  transformFilter->SetInputData(polyData);
  transformFilter->SetTransform(transform);
  transformFilter->Update();

  double bounds[6];
  transformFilter->GetOutput()->GetBounds(bounds);

  // One voxel of margin keeps voxels that the stencil tolerance includes just outside the bounds.
  int extent[6];
  for (int i = 0; i < 3; ++i)
  {
    const double first = std::max(0.0, std::ceil(bounds[2 * i]) - 1.0);
    const double last = std::min(image->GetDimension(i) - 1.0, std::floor(bounds[2 * i + 1]) + 1.0);

    if (first > last)
      return;

    extent[2 * i] = static_cast<int>(first);
    extent[2 * i + 1] = static_cast<int>(last);
  }

  auto stencilSource = vtkSmartPointer<vtkPolyDataToImageStencil>::New();
  stencilSource->SetInputConnection(transformFilter->GetOutputPort());
  stencilSource->SetTolerance(tolerance);
  stencilSource->SetOutputOrigin(0.0, 0.0, 0.0);
  stencilSource->SetOutputSpacing(1.0, 1.0, 1.0);
  stencilSource->SetOutputWholeExtent(extent);
  stencilSource->Update();

  auto* stencil = stencilSource->GetOutput();

  itk::MultiThreaderBase::New()->ParallelizeArray(extent[4], extent[5] + 1, [&](itk::SizeValueType z)
    {
      for (int y = extent[2]; y <= extent[3]; ++y)
      {
        int iter = 0;
        int first = 0;
        int last = 0;

        while (0 != stencil->GetNextExtent(first, last, extent[0], extent[1], y, static_cast<int>(z), iter))
          visitRun(static_cast<std::size_t>(first), static_cast<std::size_t>(y), z, static_cast<std::size_t>(last - first + 1));
      }
    }, nullptr);
}

mitk::SurfaceToImageFilter::SurfaceToImageFilter()
  : m_MakeOutputBinary(false), m_UShortBinaryPixelType(false), m_BackgroundValue(-10000), m_Tolerance(0.0)
{
}

mitk::SurfaceToImageFilter::~SurfaceToImageFilter()
{
}

void mitk::SurfaceToImageFilter::GenerateInputRequestedRegion()
{
  mitk::Image *output = this->GetOutput();
  if ((output->IsInitialized() == false))
    return;

  GenerateTimeInInputRegion(output, const_cast<mitk::Image *>(this->GetImage()));
}

void mitk::SurfaceToImageFilter::GenerateOutputInformation()
{
  mitk::Image *inputImage = (mitk::Image *)this->GetImage();
  mitk::Image::Pointer output = this->GetOutput();

  itkDebugMacro(<< "GenerateOutputInformation()");

  if ((inputImage == nullptr) || (inputImage->IsInitialized() == false) || (inputImage->GetTimeGeometry() == nullptr))
    return;

  if (m_MakeOutputBinary)
  {
    if (m_UShortBinaryPixelType)
    {
      output->Initialize(mitk::MakeScalarPixelType<unsigned short>(), *inputImage->GetTimeGeometry());
    }
    else
    {
      output->Initialize(mitk::MakeScalarPixelType<unsigned char>(), *inputImage->GetTimeGeometry());
    }
  }
  else
  {
    output->Initialize(inputImage->GetPixelType(), *inputImage->GetTimeGeometry());
  }

  output->SetPropertyList(inputImage->GetPropertyList()->Clone());
}

void mitk::SurfaceToImageFilter::GenerateData()
{
  mitk::Image::ConstPointer inputImage = this->GetImage();
  mitk::Image::Pointer output = this->GetOutput();

  if (inputImage.IsNull())
    return;

  if (output->IsInitialized() == false)
    return;

  mitk::Image::RegionType outputRegion = output->GetRequestedRegion();

  int tstart = outputRegion.GetIndex(3);
  int tmax = tstart + outputRegion.GetSize(3);

  if (tmax > 0)
  {
    int t;
    for (t = tstart; t < tmax; ++t)
    {
      Stencil3DImage(t);
    }
  }
  else
  {
    Stencil3DImage(0);
  }
}

void mitk::SurfaceToImageFilter::Stencil3DImage(int time)
{
  auto* output = this->GetOutput();
  const auto* image = this->GetImage();

  const auto& pixelType = output->GetPixelType();
  const std::size_t pixelSize = pixelType.GetSize();
  const std::size_t numComponents = pixelType.GetNumberOfComponents();
  const std::size_t sizeX = output->GetDimension(0);
  const std::size_t sizeY = output->GetDimension(1);
  const std::size_t numPixels = sizeX * sizeY * output->GetDimension(2);

  ImageWriteAccessor outputAccessor(output, output->GetVolumeData(time));
  auto* outputData = static_cast<char*>(outputAccessor.GetData());

  const double background = m_MakeOutputBinary ? 0.0 : m_BackgroundValue;
  mitkPixelTypeMultiplex3(FillComponents, pixelType, outputData, numPixels * numComponents, background);

  std::optional<ImageReadAccessor> inputAccessor;
  if (!m_MakeOutputBinary)
    inputAccessor.emplace(image, image->GetVolumeData(time));

  const auto* inputData = inputAccessor.has_value() ? static_cast<const char*>(inputAccessor->GetData()) : nullptr;

  const auto timePoint = image->GetTimeGeometry()->TimeStepToTimePoint(time);
  const auto surfaceTimeStep = this->GetInput()->GetTimeGeometry()->TimePointToTimeStep(timePoint);

  ForEachVoxelRunInsideSurface(this->GetInput(), surfaceTimeStep, output, time, m_Tolerance,
    [&](std::size_t x, std::size_t y, std::size_t z, std::size_t count)
    {
      const auto offset = pixelSize * ((z * sizeY + y) * sizeX + x);

      if (m_MakeOutputBinary)
      {
        mitkPixelTypeMultiplex3(FillComponents, pixelType, outputData + offset, count * numComponents, 1.0);
      }
      else
      {
        std::copy_n(inputData + offset, count * pixelSize, outputData + offset);
      }
    });
}

const mitk::Surface *mitk::SurfaceToImageFilter::GetInput(void)
{
  if (this->GetNumberOfInputs() < 1)
  {
    return nullptr;
  }

  return static_cast<const mitk::Surface *>(this->ProcessObject::GetInput(0));
}

void mitk::SurfaceToImageFilter::SetInput(const mitk::Surface *input)
{
  // Process object is not const-correct so the const_cast is required here
  this->ProcessObject::SetNthInput(0, const_cast<mitk::Surface *>(input));
}

void mitk::SurfaceToImageFilter::SetImage(const mitk::Image *source)
{
  this->ProcessObject::SetNthInput(1, const_cast<mitk::Image *>(source));
}

const mitk::Image *mitk::SurfaceToImageFilter::GetImage(void)
{
  return static_cast<const mitk::Image *>(this->ProcessObject::GetInput(1));
}

mitk::Image::Pointer mitk::ConvertSurfaceToLabelMask(const mitk::Image* refImage, const mitk::Surface* surface)
{
  if (nullptr == refImage)
    mitkThrow() << "Cannot convert to label mask. Passed reference image is nullptr.";
  if (nullptr == surface)
    mitkThrow() << "Cannot convert to label mask. Passed input is nullptr.";

  mitk::SurfaceToImageFilter::Pointer surfaceToImageFilter = mitk::SurfaceToImageFilter::New();
  surfaceToImageFilter->MakeOutputBinaryOn();
  surfaceToImageFilter->UShortBinaryPixelTypeOn();
  surfaceToImageFilter->SetInput(surface);
  surfaceToImageFilter->SetImage(refImage);

  try
  {
    surfaceToImageFilter->Update();
  }
  catch (const itk::ExceptionObject& excpt)
  {
    MITK_ERROR << "Surface conversion failed: " << excpt.GetDescription();
    return nullptr;
  }

  return surfaceToImageFilter->GetOutput();
}
