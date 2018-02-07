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

#include <mitkExtractSliceFilter2.h>
#include <mitkExceptionMacro.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageWriteAccessor.h>

#include <itkBSplineInterpolateImageFunction.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkNearestNeighborInterpolateImageFunction.h>

#include <limits>

struct mitk::ExtractSliceFilter2::Impl
{
  Impl();
  ~Impl();

  PlaneGeometry::Pointer OutputGeometry;
  mitk::ExtractSliceFilter2::Interpolator Interpolator;
  itk::Object::Pointer InterpolateImageFunction;
};

mitk::ExtractSliceFilter2::Impl::Impl()
  : Interpolator(NearestNeighbor)
{
}

mitk::ExtractSliceFilter2::Impl::~Impl()
{
}

namespace
{
  template <class TInputImage>
  void CreateInterpolateImageFunction(const TInputImage* inputImage, mitk::ExtractSliceFilter2::Interpolator interpolator, itk::Object::Pointer& result)
  {
    typename itk::InterpolateImageFunction<TInputImage>::Pointer interpolateImageFunction;

    switch (interpolator)
    {
      case mitk::ExtractSliceFilter2::NearestNeighbor:
        interpolateImageFunction = itk::NearestNeighborInterpolateImageFunction<TInputImage>::New().GetPointer();
        break;

      case mitk::ExtractSliceFilter2::Linear:
        interpolateImageFunction = itk::LinearInterpolateImageFunction<TInputImage>::New().GetPointer();
        break;

      case mitk::ExtractSliceFilter2::Cubic:
      {
        auto bSplineInterpolateImageFunction = itk::BSplineInterpolateImageFunction<TInputImage>::New();
        bSplineInterpolateImageFunction->SetSplineOrder(2);
        interpolateImageFunction = bSplineInterpolateImageFunction.GetPointer();
        break;
      }

      default:
        mitkThrow() << "Interplator is unknown.";
    }

    interpolateImageFunction->SetInputImage(inputImage);

    result = interpolateImageFunction.GetPointer();
  }

  template <typename TPixel, unsigned int VImageDimension>
  void GenerateData(const itk::Image<TPixel, VImageDimension>* inputImage, mitk::Image* outputImage, const mitk::ExtractSliceFilter2::OutputImageRegionType& outputRegion, itk::Object* interpolateImageFunction)
  {
    typedef itk::Image<TPixel, VImageDimension> TInputImage;
    typedef itk::InterpolateImageFunction<TInputImage> TInterpolateImageFunction;

    auto outputGeometry = outputImage->GetSlicedGeometry()->GetPlaneGeometry(0);
    auto interpolator = static_cast<TInterpolateImageFunction*>(interpolateImageFunction);

    auto origin = outputGeometry->GetOrigin();
    auto spacing = outputGeometry->GetSpacing();
    auto xDirection = outputGeometry->GetAxisVector(0);
    auto yDirection = outputGeometry->GetAxisVector(1);

    xDirection.Normalize();
    yDirection.Normalize();

    auto spacingAlongXDirection = xDirection * spacing[0];
    auto spacingAlongYDirection = yDirection * spacing[1];

    const std::size_t pixelSize = outputImage->GetPixelType().GetSize();
    const std::size_t width = outputGeometry->GetExtent(0);
    const std::size_t xBegin = outputRegion.GetIndex(0);
    const std::size_t yBegin = outputRegion.GetIndex(1);
    const std::size_t xEnd = xBegin + outputRegion.GetSize(0);
    const std::size_t yEnd = yBegin + outputRegion.GetSize(1);

    mitk::ImageWriteAccessor writeAccess(outputImage, nullptr, mitk::ImageAccessorBase::IgnoreLock);
    auto data = static_cast<char*>(writeAccess.GetData());

    const TPixel backgroundPixel = std::numeric_limits<TPixel>::lowest();
    TPixel pixel;

    itk::ContinuousIndex<mitk::ScalarType, 3> index;
    mitk::Point3D yPoint;
    mitk::Point3D point;

    for (std::size_t y = yBegin; y < yEnd; ++y)
    {
      yPoint = origin + spacingAlongYDirection * y;

      for (std::size_t x = xBegin; x < xEnd; ++x)
      {
        point = yPoint + spacingAlongXDirection * x;

        if (inputImage->TransformPhysicalPointToContinuousIndex(point, index))
        {
          pixel = interpolator->EvaluateAtContinuousIndex(index);
          memcpy(static_cast<void*>(data + pixelSize * (width * y + x)), static_cast<const void*>(&pixel), pixelSize);
        }
        else
        {
          memcpy(static_cast<void*>(data + pixelSize * (width * y + x)), static_cast<const void*>(&backgroundPixel), pixelSize);
        }
      }
    }
  }

  void VerifyInputImage(const mitk::Image* inputImage)
  {
    auto dimension = inputImage->GetDimension();

    if (3 != dimension)
      mitkThrow() << "Input images with " << dimension << " dimensions are not supported.";

    if (!inputImage->IsInitialized())
      mitkThrow() << "Input image is not initialized.";

    if (!inputImage->IsVolumeSet())
      mitkThrow() << "Input image volume is not set.";

    auto geometry = inputImage->GetGeometry();

    if (nullptr == geometry || !geometry->IsValid())
      mitkThrow() << "Input image has invalid geometry.";

    if (!geometry->GetImageGeometry())
      mitkThrow() << "Geometry of input image is not an image geometry.";
  }

  void VerifyOutputGeometry(const mitk::PlaneGeometry* outputGeometry)
  {
    if (nullptr == outputGeometry)
      mitkThrow() << "Output geometry is not set.";

    if (!outputGeometry->GetImageGeometry())
      mitkThrow() << "Output geometry is not an image geometry.";
  }
}

mitk::ExtractSliceFilter2::ExtractSliceFilter2()
  : m_Impl(new Impl)
{
}

mitk::ExtractSliceFilter2::~ExtractSliceFilter2()
{
  delete m_Impl;
}

void mitk::ExtractSliceFilter2::AllocateOutputs()
{
  const auto* inputImage = this->GetInput();
  const auto* outputGeometry = this->GetOutputGeometry();
  auto outputImage = this->GetOutput();
  auto pixelType = inputImage->GetPixelType();

  outputImage->Initialize(pixelType, 1, *outputGeometry);

  auto data = new char[static_cast<std::size_t>(pixelType.GetSize() * outputGeometry->GetExtent(0) * outputGeometry->GetExtent(1))];

  try
  {
    if (!outputImage->SetImportVolume(data, 0, 0, mitk::Image::ReferenceMemory))
      throw;
  }
  catch (...)
  {
    delete[] data;
  }
}

/*void mitk::ExtractSliceFilter2::BeforeThreadedGenerateData()
{
  if (nullptr != m_Impl->InterpolateImageFunction && this->GetInput()->GetMTime() < this->GetMTime())
    return;

  const auto* inputImage = this->GetInput();
  AccessFixedDimensionByItk_2(inputImage, CreateInterpolateImageFunction, 3, this->GetInterpolator(), m_Impl->InterpolateImageFunction);
}

void mitk::ExtractSliceFilter2::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, itk::ThreadIdType)
{
  const auto* inputImage = this->GetInput();
  AccessFixedDimensionByItk_3(inputImage, ::GenerateData, 3, this->GetOutput(), outputRegionForThread, m_Impl->InterpolateImageFunction);
}*/

void mitk::ExtractSliceFilter2::GenerateData()
{
  if (nullptr != m_Impl->InterpolateImageFunction && this->GetInput()->GetMTime() < this->GetMTime())
    return;

  const auto* inputImage = this->GetInput();

  AccessFixedDimensionByItk_2(inputImage, CreateInterpolateImageFunction, 3, this->GetInterpolator(), m_Impl->InterpolateImageFunction);

  this->AllocateOutputs();
  auto outputRegion = this->GetOutput()->GetLargestPossibleRegion();

  AccessFixedDimensionByItk_3(inputImage, ::GenerateData, 3, this->GetOutput(), outputRegion, m_Impl->InterpolateImageFunction);
}

void mitk::ExtractSliceFilter2::SetInput(const InputImageType* image)
{
  if (this->GetInput() == image)
    return;

  Superclass::SetInput(image);
  m_Impl->InterpolateImageFunction = nullptr;
}

void mitk::ExtractSliceFilter2::SetInput(unsigned int index, const InputImageType* image)
{
  if (0 != index)
    mitkThrow() << "Input index " << index << " is invalid.";

  this->SetInput(image);
}

const mitk::PlaneGeometry* mitk::ExtractSliceFilter2::GetOutputGeometry() const
{
  return m_Impl->OutputGeometry;
}

void mitk::ExtractSliceFilter2::SetOutputGeometry(PlaneGeometry::Pointer outputGeometry)
{
  if (m_Impl->OutputGeometry != outputGeometry)
  {
    m_Impl->OutputGeometry = outputGeometry;
    this->Modified();
  }
}

mitk::ExtractSliceFilter2::Interpolator mitk::ExtractSliceFilter2::GetInterpolator() const
{
  return m_Impl->Interpolator;
}

void mitk::ExtractSliceFilter2::SetInterpolator(Interpolator interpolator)
{
  if (m_Impl->Interpolator != interpolator)
  {
    m_Impl->Interpolator = interpolator;
    m_Impl->InterpolateImageFunction = nullptr;
    this->Modified();
  }
}

void mitk::ExtractSliceFilter2::VerifyInputInformation()
{
  Superclass::VerifyInputInformation();

  VerifyInputImage(this->GetInput());
  VerifyOutputGeometry(this->GetOutputGeometry());
}
