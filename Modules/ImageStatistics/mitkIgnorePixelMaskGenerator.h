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
 * @brief The IgnorePixelMaskGenerator class is used to generate a mask that is zero for specific pixel values in the input image. This class requires an input image.
 */
class MITKIMAGESTATISTICS_EXPORT IgnorePixelMaskGenerator: public MaskGenerator
{
public:
    /** Standard Self typedef */
    typedef IgnorePixelMaskGenerator            Self;
    typedef MaskGenerator                       Superclass;
    typedef itk::SmartPointer< Self >           Pointer;
    typedef itk::SmartPointer< const Self >     ConstPointer;
    typedef double RealType;

    /** Method for creation through the object factory. */
    itkNewMacro(Self); /** Runtime information support. */
    itkTypeMacro(IgnorePixelMaskGenerator, MaskGenerator);

    /**
     * @brief The mask will be 0 there inputImage==pixelValue and 1 otherwise
     */
    void SetIgnoredPixelValue(RealType pixelValue);

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
