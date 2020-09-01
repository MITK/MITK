/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef MITK_IMAGE_STITCHING_HELPER_H
#define MITK_IMAGE_STITCHING_HELPER_H

#include "mapRegistrationBase.h"
#include "mitkImage.h"
#include "mitkGeometry3D.h"

#include "mitkMAPRegistrationWrapper.h"
#include "mitkImageMappingHelper.h"

#include <itkStitchImageFilter.h>

#include "MitkMatchPointRegistrationExports.h"

namespace mitk
{
    /**Helper that stitches a given vector of input images
     * @param inputs vector of input images that should be stitched.
     * @param registrations vector of registrations that should be used for mapping of the inputs before stitching.
     * the method assumes that order of registrations is the same as the order of inputs, thus for the n-th input
     * the n-th registration will be used.
     * @param resultGeometry Pointer to the Geometry object that specifies the grid of the result image.
     * @param paddingValue Indicates the value that should be used if an out of input error occurs (and throwOnOutOfInputAreaError is false).
     * @param interpolatorType Indicates the type of interpolation strategy that should be used.
     * @param stitchStrategy Strategy used if more than one input can contribute. for more details see the documentation of itk::StitchStrategy.
     * @pre inputs must not be empty and contain valid instances
     * @pre registration must have same size as inputs and contain valid instances.
     * @pre Dimensionality of the registrations must match with the inputs
     * @pre resultGeometry must be valid.
     * @remark The helper currently only supports 3D images.
     * @result Pointer to the resulting mapped image.h*/
    MITKMATCHPOINTREGISTRATION_EXPORT Image::Pointer StitchImages(std::vector<Image::ConstPointer> inputs,
      std::vector<::map::core::RegistrationBase::ConstPointer> registrations,
      const BaseGeometry* resultGeometry,
      const double& paddingValue = 0, itk::StitchStrategy stitchStrategy = itk::StitchStrategy::Mean,
      mitk::ImageMappingInterpolator::Type interpolatorType = mitk::ImageMappingInterpolator::Linear);

    MITKMATCHPOINTREGISTRATION_EXPORT Image::Pointer StitchImages(std::vector<Image::ConstPointer> inputs,
      std::vector<MAPRegistrationWrapper::ConstPointer> registrations,
      const BaseGeometry* resultGeometry,
      const double& paddingValue = 0, itk::StitchStrategy stitchStrategy = itk::StitchStrategy::Mean,
      mitk::ImageMappingInterpolator::Type interpolatorType = mitk::ImageMappingInterpolator::Linear);

    /**@overload
     * Convinience version that uses identity transforms form the registrations.
     */
    MITKMATCHPOINTREGISTRATION_EXPORT Image::Pointer StitchImages(std::vector<Image::ConstPointer> inputs,
      const BaseGeometry* resultGeometry,
      const double& paddingValue = 0, itk::StitchStrategy stitchStrategy = itk::StitchStrategy::Mean,
      mitk::ImageMappingInterpolator::Type interpolatorType = mitk::ImageMappingInterpolator::Linear);

}

#endif
