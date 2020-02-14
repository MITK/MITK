/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKIGNOREPIXELMASKGEN_
#define MITKIGNOREPIXELMASKGEN_

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

    /**
     * @brief Computes and returns the mask
     */
    mitk::Image::Pointer GetMask() override;

    /**
     * @brief SetTimeStep is used to set the time step for which the mask is to be generated
     * @param timeStep
     */
    void SetTimeStep(unsigned int timeStep) override;

protected:
    IgnorePixelMaskGenerator():
       m_IgnoredPixelValue(std::numeric_limits<RealType>::min())
    {
        m_TimeStep = 0;
        m_InternalMaskUpdateTime = 0;
        m_InternalMask = mitk::Image::New();
    }

    ~IgnorePixelMaskGenerator() override{}

    template <typename TPixel, unsigned int VImageDimension>
    void InternalCalculateMask(typename itk::Image<TPixel, VImageDimension>* image);

private:
    bool IsUpdateRequired() const;

    RealType m_IgnoredPixelValue;
    unsigned long m_InternalMaskUpdateTime;


};

}

#endif
