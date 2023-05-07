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
 * @brief Utility class for mask operations. It checks whether an image and a mask are compatible (spacing, orientation, etc...)
 * and it can also crop an image to the LargestPossibleRegion of the Mask
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

        typedef itk::Image<TPixel, VImageDimension> ImageType;
        typedef itk::Image<unsigned short, VImageDimension> MaskType;

        /**
         * @brief Set image
         */
        void SetImage(const ImageType* image);

        /**
         * @brief Set mask
         */
        void SetMask(const MaskType* mask);

        /**
         * @brief Checks whether mask and image are compatible for joint access (as via iterators).
         * Spacing and direction must be the same between the two and they must be aligned. Also, the mask must be completely inside the image
         */
        bool CheckMaskSanity();

        /**
         * @brief Crops the image to the LargestPossibleRegion of the mask
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
 * coordinate aspects (orgin, size, grid alignment).*/
constexpr double MASK_SUITABILITY_TOLERANCE_COORDINATE = NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION;
/** Tolerance used to check if the mask and input image are compatible for
 * direction aspects (orientation of mask and image).*/
constexpr double MASK_SUITABILITY_TOLERANCE_DIRECTION = NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_DIRECTION_PRECISION;

}

#ifndef ITK_MANUAL_INSTANTIATION
#include <mitkMaskUtilities.tpp>
#endif

#endif
