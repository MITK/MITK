/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageMaskGenerator_h
#define mitkImageMaskGenerator_h

#include <mitkImage.h>
#include <MitkImageStatisticsExports.h>
#include <mitkMaskGenerator.h>
#include <itkObject.h>
#include <itkSmartPointer.h>

namespace mitk
{
/**
 * \brief Generates a mask from an existing mitk::Image for use in statistics calculations.
 *
 * This mask generator takes an external binary or label image (set via
 * SetImageMask()) and provides it as a mask for the ImageStatisticsCalculator.
 * The mask image is internally adapted (e.g., time step extraction) to match
 * the input image geometry.
 *
 * \sa MaskGenerator
 * \sa ImageStatisticsCalculator
 */
class MITKIMAGESTATISTICS_EXPORT ImageMaskGenerator: public MaskGenerator
{
public:
    /** Standard Self typedef */
    typedef ImageMaskGenerator            Self;
    typedef MaskGenerator                       Superclass;
    typedef itk::SmartPointer< Self >           Pointer;
    typedef itk::SmartPointer< const Self >     ConstPointer;

    /** Method for creation through the object factory. */
    itkNewMacro(Self); /** Runtime information support. */
    itkTypeMacro(ImageMaskGenerator, MaskGenerator);

    /**
     * \brief Returns the number of masks this generator provides.
     * \return Always returns 1.
     */
    unsigned int GetNumberOfMasks() const override;

    /**
     * \brief Set the image mask to use.
     * \param[in] _arg Const pointer to the mask image. Non-zero pixels define
     *            the region of interest.
     */
    itkSetConstObjectMacro(ImageMask, Image)

protected:
    ImageMaskGenerator():Superclass(){
        m_InternalMaskUpdateTime = 0;
    }

    Image::ConstPointer DoGetMask(unsigned int) override;

private:
    bool IsUpdateRequired() const;
    void UpdateInternalMask();

    mitk::Image::ConstPointer m_ImageMask;
    mitk::Image::ConstPointer m_InternalMask;
    unsigned long m_InternalMaskUpdateTime;

};


}

#endif
