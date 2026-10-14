/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIgnorePixelMaskGenerator.h>
#include <mitkImageReadAccessor.h>
#include <mitkImageTimeSelector.h>
#include <mitkImageWriteAccessor.h>
#include <mitkPixelTypeMultiplex.h>

#include <algorithm>

namespace
{
  /** Sets a mask voxel to 0 if all compared components of the image voxel have the ignored
  value, and to 1 otherwise. Reads the raw components, so it covers every kind of pixel, e.g.
  RGB of any component type, vectors, or tensors. */
  template <typename TComponent>
  void CalculateMask(const mitk::PixelType& pixelType, const mitk::Image* image, double ignoredValue, mitk::Image::Pointer& mask)
  {
    std::size_t numberOfVoxels = 1;
    for (unsigned int i = 0; i < image->GetDimension(); ++i)
      numberOfVoxels *= image->GetDimension(i);

    const auto numberOfComponents = pixelType.GetNumberOfComponents();

    // The alpha is no color, so black RGBA voxels are ignored regardless of their opacity.
    const auto numberOfComparedComponents = itk::IOPixelEnum::RGBA == pixelType.GetPixelType()
      ? 3u
      : numberOfComponents;

    const auto ignoredComponentValue = static_cast<TComponent>(ignoredValue);
    const auto isIgnored = [ignoredComponentValue](TComponent component) { return component == ignoredComponentValue; };

    auto result = mitk::Image::New();
    result->Initialize(mitk::MakeScalarPixelType<unsigned short>(), image->GetDimension(), image->GetDimensions());

    {
      mitk::ImageReadAccessor imageAccessor(image);
      mitk::ImageWriteAccessor resultAccessor(result);

      const auto* voxel = static_cast<const TComponent*>(imageAccessor.GetData());
      auto* maskVoxel = static_cast<unsigned short*>(resultAccessor.GetData());

      for (std::size_t i = 0; i < numberOfVoxels; ++i, voxel += numberOfComponents)
        maskVoxel[i] = std::all_of(voxel, voxel + numberOfComparedComponents, isIgnored) ? 0 : 1;
    }

    mask = result;
  }
}

namespace mitk
{
void IgnorePixelMaskGenerator::SetIgnoredPixelValue(RealType pixelValue)
{
    if (pixelValue != m_IgnoredPixelValue)
    {
        m_IgnoredPixelValue = pixelValue;
        this->Modified();
    }
}

unsigned int IgnorePixelMaskGenerator::GetNumberOfMasks() const
{
  return 1;
}

mitk::Image::ConstPointer IgnorePixelMaskGenerator::DoGetMask(unsigned int)
{
    if (IsUpdateRequired())
    {
        if (m_InputImage.IsNull())
        {
            mitkThrow() << "Image not set!";
        }

        if (m_IgnoredPixelValue == std::numeric_limits<RealType>::min())
        {
          mitkThrow() << "IgnotePixelValue not set!";
        }

        auto timeSliceImage = mitk::SelectImageByTimePoint(m_InputImage, m_TimePoint);

        if (timeSliceImage.IsNull()) mitkThrow() << "Cannot generate mask. Passed time point is not supported by input image. Invalid time point: "<< m_TimePoint;

        const auto pixelType = timeSliceImage->GetPixelType();
        mitk::Image::Pointer mask;
        mitkPixelTypeMultiplex3(CalculateMask, pixelType, timeSliceImage.GetPointer(), m_IgnoredPixelValue, mask);

        // mitkPixelTypeMultiplex calls nothing for a component type it does not know.
        if (mask.IsNull())
          mitkThrow() << "Cannot generate mask. Unsupported component type: " << pixelType.GetComponentTypeAsString();

        m_InternalMask = mask;
        m_InternalMask->SetGeometry(timeSliceImage->GetGeometry());

        this->Modified();
    }
    m_InternalMaskUpdateTime = m_InternalMask->GetMTime();
    return m_InternalMask;
}

bool IgnorePixelMaskGenerator::IsUpdateRequired() const
{
    unsigned long thisClassTimeStamp = this->GetMTime();
    unsigned long internalMaskTimeStamp = m_InternalMask.IsNull() ? 0 : m_InternalMask->GetMTime();
    unsigned long inputImageTimeStamp = m_InputImage->GetMTime();

    if (thisClassTimeStamp > m_InternalMaskUpdateTime) // inputs have changed
    {
        return true;
    }

    if (m_InternalMaskUpdateTime < inputImageTimeStamp) // mask image has changed outside of this class
    {
        return true;
    }

    if (internalMaskTimeStamp > m_InternalMaskUpdateTime) // internal mask has been changed outside of this class
    {
        return true;
    }

    return false;
}

} // end namespace
