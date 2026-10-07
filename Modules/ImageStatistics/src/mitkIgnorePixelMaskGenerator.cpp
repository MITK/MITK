/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIgnorePixelMaskGenerator.h>
#include <mitkImageTimeSelector.h>
#include <mitkImageAccessByItk.h>
#include <itkDefaultConvertPixelTraits.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>
#include <itkRGBAPixel.h>
#include <mitkITKImageImport.h>

#include <type_traits>

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

        // update m_InternalMask
        switch (timeSliceImage->GetPixelType().GetPixelType())
        {
          case itk::IOPixelEnum::SCALAR:
            AccessByItk(timeSliceImage, InternalCalculateMask);
            break;

          case itk::IOPixelEnum::VECTOR:
            AccessVectorPixelTypeByItk(timeSliceImage, InternalCalculateMask);
            break;

          default:
            AccessFixedPixelTypeByItk(timeSliceImage, InternalCalculateMask, MITK_ACCESSBYITK_COMPOSITE_PIXEL_TYPES_SEQ);
            break;
        }

        m_InternalMask->SetGeometry(timeSliceImage->GetGeometry());

        this->Modified();
    }
    m_InternalMaskUpdateTime = m_InternalMask->GetMTime();
    return m_InternalMask;
}

template <typename TImage>
void IgnorePixelMaskGenerator::InternalCalculateMask(const TImage* image)
{
    using PixelType = typename TImage::PixelType;
    using PixelTraits = itk::DefaultConvertPixelTraits<PixelType>;
    using ComponentType = typename PixelTraits::ComponentType;
    using MaskType = itk::Image<unsigned short, TImage::ImageDimension>;

    // The alpha is no color, so black RGBA voxels are ignored regardless of their opacity.
    constexpr bool isRGBA = std::is_same_v<PixelType, itk::RGBAPixel<ComponentType>>;

    typename MaskType::Pointer mask = MaskType::New();
    mask->SetOrigin(image->GetOrigin());
    mask->SetSpacing(image->GetSpacing());
    mask->SetLargestPossibleRegion(image->GetLargestPossibleRegion());
    mask->SetBufferedRegion(image->GetBufferedRegion());
    mask->SetDirection(image->GetDirection());
    mask->Allocate();
    mask->FillBuffer(1);

    const auto ignoredComponentValue = static_cast<ComponentType>(m_IgnoredPixelValue);

    // iterate over image and mask and set mask=0 if all compared components of the image pixel equal m_IgnoredPixelValue
    itk::ImageRegionConstIterator<TImage> imageIterator(image, image->GetLargestPossibleRegion());
    itk::ImageRegionIterator<MaskType> maskIterator(mask, mask->GetLargestPossibleRegion());

    for (imageIterator.GoToBegin(); !imageIterator.IsAtEnd(); ++imageIterator, ++maskIterator)
    {
        const auto pixel = imageIterator.Get();
        const unsigned int numberOfComparedComponents = isRGBA ? 3 : PixelTraits::GetNumberOfComponents(pixel);
        bool ignored = true;

        for (unsigned int i = 0; ignored && i < numberOfComparedComponents; ++i)
          ignored = PixelTraits::GetNthComponent(i, pixel) == ignoredComponentValue;

        if (ignored)
        {
            maskIterator.Set(0);
        }
    }

    m_InternalMask = GrabItkImageMemory(mask);
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
