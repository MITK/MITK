/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBinaryMaskGenerator
#define mitkBinaryMaskGenerator

#include <mitkImage.h>
#include <MitkImageStatisticsExports.h>
#include <mitkMaskGenerator.h>
#include <itkObject.h>
#include <itkSmartPointer.h>

namespace mitk
{
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
    itkTypeMacro(BinaryImageMaskGenerator, MaskGenerator);

    mitk::Image::Pointer GetMask() override;

    void SetTimeStep(unsigned int timeStep) override;

    void SetImageMask(mitk::Image::Pointer maskImage);

protected:
    ImageMaskGenerator():Superclass(){
        m_InternalMaskUpdateTime = 0;
        m_InternalMask = mitk::Image::New();
    }

private:
    bool IsUpdateRequired() const;
    void UpdateInternalMask();

    mitk::Image::Pointer m_internalMaskImage;
    unsigned long m_InternalMaskUpdateTime;

};


}

#endif
