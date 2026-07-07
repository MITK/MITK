/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkImageStitchingHelper_h
#define mitkImageStitchingHelper_h

#include <mapRegistrationBase.h>
#include <mitkImage.h>
#include <mitkGeometry3D.h>

#include <mitkMAPRegistrationWrapper.h>
#include <mitkImageMappingHelper.h>

#include <itkStitchImageFilter.h>

#include <MitkMatchPointRegistrationExports.h>

/**
 * \brief Helper functions for stitching multiple images into a single result image using MatchPoint registrations.
 *
 * \sa itk::StitchImageFilter, mitk::ImageMappingHelper
 */
namespace mitk
{
    /**
     * \brief Stitches multiple input images into a single result image using MatchPoint registrations.
     *
     * Each input image is mapped into the result geometry using its corresponding registration.
     * When multiple inputs contribute to the same output voxel, the stitch strategy determines
     * how values are combined.
     *
     * \param[in] inputs Vector of input images to stitch.
     * \param[in] registrations Vector of MatchPoint registrations, one per input image (same order).
     * \param[in] resultGeometry Geometry defining the grid of the output image.
     * \param[in] paddingValue Value used for voxels not covered by any input image.
     * \param[in] stitchStrategy Strategy for combining overlapping inputs (Mean or BorderDistance).
     * \param[in] interpolatorType Interpolation strategy for resampling.
     * \return Smart pointer to the stitched result image.
     * \pre \p inputs must not be empty and must contain valid instances.
     * \pre \p registrations must have the same size as \p inputs and contain valid instances.
     * \pre Dimensionality of the registrations must match the inputs.
     * \pre \p resultGeometry must be valid.
     * \note Currently only 3D images are supported.
     */
    MITKMATCHPOINTREGISTRATION_EXPORT Image::Pointer StitchImages(std::vector<Image::ConstPointer> inputs,
      std::vector<::map::core::RegistrationBase::ConstPointer> registrations,
      const BaseGeometry* resultGeometry,
      const double& paddingValue = 0, itk::StitchStrategy stitchStrategy = itk::StitchStrategy::Mean,
      mitk::ImageMappingInterpolator::Type interpolatorType = mitk::ImageMappingInterpolator::Linear);

    /**
     * \brief Stitches multiple input images using MITK registration wrappers.
     * \overload
     *
     * \param[in] inputs Vector of input images to stitch.
     * \param[in] registrations Vector of MITK registration wrappers, one per input image.
     * \param[in] resultGeometry Geometry defining the grid of the output image.
     * \param[in] paddingValue Value used for uncovered voxels.
     * \param[in] stitchStrategy Strategy for combining overlapping inputs.
     * \param[in] interpolatorType Interpolation strategy for resampling.
     * \return Smart pointer to the stitched result image.
     */
    MITKMATCHPOINTREGISTRATION_EXPORT Image::Pointer StitchImages(std::vector<Image::ConstPointer> inputs,
      std::vector<MAPRegistrationWrapper::ConstPointer> registrations,
      const BaseGeometry* resultGeometry,
      const double& paddingValue = 0, itk::StitchStrategy stitchStrategy = itk::StitchStrategy::Mean,
      mitk::ImageMappingInterpolator::Type interpolatorType = mitk::ImageMappingInterpolator::Linear);

    /**
     * \brief Stitches multiple input images using identity transforms.
     * \overload
     *
     * Convenience version that uses identity registrations for all inputs.
     *
     * \param[in] inputs Vector of input images to stitch.
     * \param[in] resultGeometry Geometry defining the grid of the output image.
     * \param[in] paddingValue Value used for uncovered voxels.
     * \param[in] stitchStrategy Strategy for combining overlapping inputs.
     * \param[in] interpolatorType Interpolation strategy for resampling.
     * \return Smart pointer to the stitched result image.
     */
    MITKMATCHPOINTREGISTRATION_EXPORT Image::Pointer StitchImages(std::vector<Image::ConstPointer> inputs,
      const BaseGeometry* resultGeometry,
      const double& paddingValue = 0, itk::StitchStrategy stitchStrategy = itk::StitchStrategy::Mean,
      mitk::ImageMappingInterpolator::Type interpolatorType = mitk::ImageMappingInterpolator::Linear);

}

#endif
