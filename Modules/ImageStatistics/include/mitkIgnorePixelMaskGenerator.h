/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIgnorePixelMaskGenerator_h
#define mitkIgnorePixelMaskGenerator_h

#include <mitkImage.h>
#include <MitkImageStatisticsExports.h>
#include <mitkMaskGenerator.h>
#include <limits>
#include <itkImage.h>


namespace mitk
{
/**
 * \brief Generates a binary mask that excludes pixels with a specified intensity value.
 *
 * This mask generator creates a mask where every pixel is 1 except for
 * those pixels in the input image whose value matches the ignored pixel
 * value, which are set to 0. This is commonly used to exclude background
 * or zero-valued voxels from statistics calculations.
 *
 * \pre An input image must be set via SetInputImage() before calling GetMask().
 *
 * \sa MaskGenerator
 * \sa ImageStatisticsCalculator
 */
class MITKIMAGESTATISTICS_EXPORT IgnorePixelMaskGenerator: public MaskGenerator
{
public:
    /** Standard Self typedef */
    typedef IgnorePixelMaskGenerator            Self;
    typedef MaskGenerator                       Superclass;
    typedef itk::SmartPointer< Self >           Pointer;
    typedef itk::SmartPointer< const Self >     ConstPointer;
    /** \brief Real-valued type used for the pixel value to ignore. */
    typedef double RealType;

    /** Method for creation through the object factory. */
    itkNewMacro(Self); /** Runtime information support. */
    itkTypeMacro(IgnorePixelMaskGenerator, MaskGenerator);

    /**
     * \brief Set the pixel value to be excluded from the mask.
     *
     * Pixels in the input image with this value will be set to 0 in the
     * generated mask; all other pixels will be set to 1.
     *
     * \param[in] pixelValue The intensity value to ignore.
     */
    void SetIgnoredPixelValue(RealType pixelValue);

    /**
     * \brief Returns the number of masks this generator provides.
     * \return Always returns 1.
     */
    unsigned int GetNumberOfMasks() const override;

protected:
    IgnorePixelMaskGenerator():
       m_IgnoredPixelValue(std::numeric_limits<RealType>::min()), m_InternalMaskUpdateTime(0)
    {
    }

    ~IgnorePixelMaskGenerator() = default;

    mitk::Image::ConstPointer DoGetMask(unsigned int maskID) override;

    template <typename TPixel, unsigned int VImageDimension>
    void InternalCalculateMask(const itk::Image<TPixel, VImageDimension>* image);

private:
    bool IsUpdateRequired() const;

    mitk::Image::Pointer m_InternalMask;
    RealType m_IgnoredPixelValue;
    unsigned long m_InternalMaskUpdateTime;


};

}

#endif
