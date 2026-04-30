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
 * \brief Abstract base class for all mask generators used in image statistics.
 *
 * Mask generators create binary (unsigned short) masks that define regions of
 * interest for image statistics calculations. Derived classes implement the
 * actual mask creation logic in DoGetMask().
 *
 * A mask generator may optionally require an input image (set via
 * SetInputImage()) and a time point (set via SetTimePoint()). The public
 * GetMask() method delegates to the pure virtual DoGetMask() after any
 * necessary validation.
 *
 * \sa ImageStatisticsCalculator
 * \sa ImageMaskGenerator
 * \sa PlanarFigureMaskGenerator
 * \sa IgnorePixelMaskGenerator
 * \sa HotspotMaskGenerator
 * \sa MultiLabelMaskGenerator
 */
class MITKIMAGESTATISTICS_EXPORT MaskGenerator: public itk::Object
{
public:
    mitkClassMacroItkParent(MaskGenerator, itk::Object);

    /**
     * \brief Get the number of masks this generator can produce.
     * \return The number of available masks.
     */
    virtual unsigned int GetNumberOfMasks() const = 0;

    /**
     * \brief Get the generated mask image.
     * \param[in] maskID Index of the mask to retrieve (for generators that produce multiple masks).
     * \return Const pointer to the generated mask image.
     */
    mitk::Image::ConstPointer GetMask(unsigned int maskID);

    /**
     * \brief Get the reference image associated with this mask generator.
     *
     * By default, returns the input image (as set by SetInputImage()). If no
     * input image is set, returns nullptr. Subclasses may override this to
     * return a different reference image.
     *
     * \return Const pointer to the reference image, or nullptr.
     */
    virtual mitk::Image::ConstPointer GetReferenceImage();

    /**
     * \brief Set the input image for the mask generator.
     *
     * Some subclasses require an input image to generate the mask (e.g.,
     * IgnorePixelMaskGenerator). Others do not. See the documentation of
     * the specific subclass.
     *
     * \param[in] _arg Const pointer to the input image.
     */
    itkSetConstObjectMacro(InputImage, mitk::Image);

    /**
     * \brief Set the time point for which to generate the mask.
     * \param[in] _arg The time point value.
     */
    itkSetMacro(TimePoint, TimePointType);

protected:
    MaskGenerator();

    /**
     * \brief Must be overridden by derived classes to generate the requested mask.
     * \param[in] maskID Parameter indicating which mask should be returned.
     * \return The generated mask image.
     */
    virtual mitk::Image::ConstPointer DoGetMask(unsigned int maskID) = 0;


    TimePointType m_TimePoint;
    mitk::Image::ConstPointer m_InputImage;

private:

};
}

#endif
