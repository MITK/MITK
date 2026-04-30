/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMaskUtilities_h
#define mitkMaskUtilities_h

#include <MitkImageStatisticsExports.h>
#include <mitkImage.h>
#include <mitkNodePredicateGeometry.h>
#include <itkImage.h>

namespace mitk
{
/**
 * \brief Utility class for validating and adapting masks for use with images.
 *
 * MaskUtilities provides functionality to check whether an ITK image and a
 * mask image are geometrically compatible (same spacing, direction, and
 * alignment) and to crop the image region to match the mask's
 * LargestPossibleRegion.
 *
 * \tparam TPixel The pixel type of the image.
 * \tparam VImageDimension The dimensionality of the image and mask.
 *
 * \sa ImageStatisticsCalculator
 * \sa ImageMaskGenerator
 */
template <class TPixel, unsigned int VImageDimension>
class MaskUtilities: public itk::Object
    {
    public:
        /** Standard Self typedef */
        typedef MaskUtilities                       Self;
        typedef itk::Object                         Superclass;
        typedef itk::SmartPointer< Self >           Pointer;
        typedef itk::SmartPointer< const Self >     ConstPointer;

        /** Method for creation through the object factory. */
        itkNewMacro(Self); /** Runtime information support. */
        itkTypeMacro(MaskUtilities, itk::Object);

        /** \brief ITK image type with the given pixel type and dimension. */
        typedef itk::Image<TPixel, VImageDimension> ImageType;
        /** \brief Mask image type (unsigned short) with the given dimension. */
        typedef itk::Image<unsigned short, VImageDimension> MaskType;

        /**
         * \brief Set the image to validate against the mask.
         * \param[in] image Const pointer to the ITK image.
         */
        void SetImage(const ImageType* image);

        /**
         * \brief Set the mask image.
         * \param[in] mask Const pointer to the ITK mask image.
         */
        void SetMask(const MaskType* mask);

        /**
         * \brief Check whether the mask and image are geometrically compatible.
         *
         * Validates that spacing and direction are the same between image and
         * mask (within tolerance), that the two are grid-aligned, and that the
         * mask region is completely inside the image region.
         *
         * \return True if mask and image are compatible for joint pixel access.
         *
         * \sa MASK_SUITABILITY_TOLERANCE_COORDINATE
         * \sa MASK_SUITABILITY_TOLERANCE_DIRECTION
         */
        bool CheckMaskSanity();

        /**
         * \brief Crop the image to the LargestPossibleRegion of the mask.
         *
         * \return Const pointer to the cropped image region matching the mask extent.
         */
        typename ImageType::ConstPointer ExtractMaskImageRegion();

    protected:
        MaskUtilities(): m_Image(nullptr), m_Mask(nullptr){}

        ~MaskUtilities() override{}

    private:
        const ImageType* m_Image;
        const MaskType* m_Mask;
    };

/** Tolerance used to check if the mask and input image are compatible for
 * coordinate aspects (origin, size, grid alignment).*/
constexpr double MASK_SUITABILITY_TOLERANCE_COORDINATE = NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION;
/** Tolerance used to check if the mask and input image are compatible for
 * direction aspects (orientation of mask and image).*/
constexpr double MASK_SUITABILITY_TOLERANCE_DIRECTION = NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_DIRECTION_PRECISION;

}

#ifndef ITK_MANUAL_INSTANTIATION
#include <mitkMaskUtilities.tpp>
#endif

#endif
