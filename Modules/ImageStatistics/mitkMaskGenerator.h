/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMaskGenerator_h
#define mitkMaskGenerator_h

#include <MitkImageStatisticsExports.h>
#include <mitkImage.h>
#include <itkRegion.h>
#include <itkObject.h>
#include <itkSmartPointer.h>

namespace mitk
{
/**
* \class MaskGenerator
* \brief Base Class for all Mask Generators. Mask generators are classes that provide functionality for the
* creation of binary (or unsigned short) masks that can be applied to an image. See dervied classes for more
* information.
*/
class MITKIMAGESTATISTICS_EXPORT MaskGenerator: public itk::Object
{
public:
    mitkClassMacroItkParent(MaskGenerator, itk::Object);

    virtual unsigned int GetNumberOfMasks() const = 0;
    /**
     * @brief GetMask returns the requested (multi) label mask.
     * @param maskID Parameter indicating which mask should be returned.
     * @return mitk::Image::Pointer of generated mask requested
     */
    mitk::Image::ConstPointer GetMask(unsigned int maskID);

    /**
     * @brief GetReferenceImage per default returns the inputImage (as set by SetInputImage). If no input image is set it will return a nullptr.
     */
    virtual mitk::Image::ConstPointer GetReferenceImage();

    /**
     * @brief SetInputImage is used to set the input image to the mask generator. Some subclasses require an input image, others don't. See the documentation of the specific Mask Generator for more information.
     */
    itkSetConstObjectMacro(InputImage, mitk::Image);
    itkSetMacro(TimePoint, TimePointType);

protected:
    MaskGenerator();

    /**
     * @brief DoGetMask must be overridden by derived classes.
     * @param maskID Parameter indicating which mask should be returned.
     * @return mitk::Image::Pointer of generated mask requested
     */
    virtual mitk::Image::ConstPointer DoGetMask(unsigned int maskID) = 0;


    TimePointType m_TimePoint;
    mitk::Image::ConstPointer m_InputImage;

private:

};
}

#endif
