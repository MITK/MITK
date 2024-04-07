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

    unsigned int GetNumberOfMasks() const override;

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
