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
#include <mitkITKImageImport.h>

#include <itkImportImageFilter.h>
#include <itkBSplineInterpolateImageFunction.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkResampleImageFilter.h>

#include <array>
#include <limits>

namespace
{
  template <typename TInputImage>
  typename itk::InterpolateImageFunction<TInputImage>::Pointer CreateInterpolator(mitk::ExtractSliceFilter2::Interpolator interpolator)
  {
    switch (interpolator)
    {
    case mitk::ExtractSliceFilter2::Interpolator::NearestNeighbor:
      return itk::NearestNeighborInterpolateImageFunction<TInputImage>::New().GetPointer();

    case mitk::ExtractSliceFilter2::Interpolator::Bilinear:
      return itk::LinearInterpolateImageFunction<TInputImage>::New().GetPointer();

    case mitk::ExtractSliceFilter2::Interpolator::Bicubic:
    {
      auto interpolateImageFunction = itk::BSplineInterpolateImageFunction<TInputImage>::New();
      interpolateImageFunction->SetSplineOrder(2);
      return interpolateImageFunction.GetPointer();
    }

    default:
      mitkThrow() << "Interplator is unknown.";
    }
  }

  template <typename TPixel, unsigned int VImageDimension>
  void GenerateData(itk::Image<TPixel, VImageDimension>* inputImage, mitk::Image* outputImage, mitk::PlaneGeometry* outputGeometry, mitk::ExtractSliceFilter2::Interpolator interpolator)
  {
    typedef typename itk::ImportImageFilter<TPixel, VImageDimension> ImportImageFilter;
    typedef typename ImportImageFilter::IndexType Index;
    typedef typename ImportImageFilter::SizeType Size;
    typedef typename ImportImageFilter::RegionType Region;
    typedef typename Size::SizeValueType SizeValue;

    Index index;
    index.Fill(0);

    Size size;
    size[0] = outputGeometry->GetExtent(0);
    size[1] = outputGeometry->GetExtent(1);
    size[2] = 1;

    Region region;
    region.SetIndex(index);
    region.SetSize(size);

    auto interpolateImageFunction = CreateInterpolator<itk::Image<TPixel, VImageDimension>>(interpolator);
    interpolateImageFunction->SetInputImage(inputImage);

    const SizeValue NUMBER_OF_PIXELS = size[0] * size[1] * size[2];
    TPixel* buffer = new TPixel[NUMBER_OF_PIXELS];

    std::array<mitk::Vector3D, 3> direction =
    {
      outputGeometry->GetAxisVector(0),
      outputGeometry->GetAxisVector(1),
      outputGeometry->GetNormal()
    };

    direction[0].Normalize();
    direction[1].Normalize();
    direction[2].Normalize();

    mitk::Matrix3D directionMatrix;

    for (int j = 0; j < 3; ++j)
    {
      for (int i = 0; i < 3; ++i)
      {
        directionMatrix(i, j) = direction[j][i];
      }
    }

    auto spacing = outputGeometry->GetSpacing();

    std::array<mitk::Vector3D, 2> spacingAlongDirection =
    {
      direction[0] * spacing[0],
      direction[1] * spacing[1]
    };

    auto origin = outputGeometry->GetOrigin();
    origin -= spacingAlongDirection[0] * 0.5;
    origin -= spacingAlongDirection[1] * 0.5;

    mitk::Point3D point;
    mitk::Point3D yComponent;
    itk::ContinuousIndex<mitk::ScalarType, 3> continuousIndex;

    for (SizeValue y = 0; y < size[1]; ++y)
    {
      yComponent = origin + spacingAlongDirection[1] * y;

      for (SizeValue x = 0; x < size[0]; ++x)
      {
        point = yComponent + spacingAlongDirection[0] * x;

        buffer[size[1] * y + x] = inputImage->TransformPhysicalPointToContinuousIndex(point, continuousIndex)
          ? interpolateImageFunction->EvaluateAtContinuousIndex(continuousIndex)
          : std::numeric_limits<TPixel>::lowest();
      }
    }

    auto importImageFilter = ImportImageFilter::New();
    importImageFilter->SetRegion(region);
    importImageFilter->SetOrigin(outputGeometry->GetOrigin());
    importImageFilter->SetSpacing(outputGeometry->GetSpacing());
    importImageFilter->SetDirection(directionMatrix);
    importImageFilter->SetImportPointer(buffer, NUMBER_OF_PIXELS, true);
    importImageFilter->Update();

    mitk::GrabItkImageMemory(importImageFilter->GetOutput(), outputImage);
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

struct mitk::ExtractSliceFilter2::Impl
{
  Impl();
  ~Impl();

  PlaneGeometry::Pointer OutputGeometry;
  Interpolator Interpolator;
};

mitk::ExtractSliceFilter2::Impl::Impl()
  : Interpolator(Interpolator::NearestNeighbor)
{
}

mitk::ExtractSliceFilter2::Impl::~Impl()
{
}

mitk::ExtractSliceFilter2::ExtractSliceFilter2()
  : m_Impl(new Impl)
{
}

mitk::ExtractSliceFilter2::~ExtractSliceFilter2()
{
  delete m_Impl;
}

void mitk::ExtractSliceFilter2::GenerateData()
{
  AccessFixedDimensionByItk_n(this->GetInput(), ::GenerateData, 3, (this->GetOutput(), m_Impl->OutputGeometry, m_Impl->Interpolator));
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
    this->Modified();
  }
}

void mitk::ExtractSliceFilter2::VerifyInputInformation()
{
  Superclass::VerifyInputInformation();

  VerifyInputImage(this->GetInput());
  VerifyOutputGeometry(m_Impl->OutputGeometry.GetPointer());
}
