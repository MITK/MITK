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

#include "MitkMatchPointRegistrationExports.h"

namespace mitk
{
    /**Helper that maps a given input image
     * @param input Image that should be mapped.
     * @param registration Pointer to the registration instance that should be used for mapping
     * @param resultGeometry Pointer to the Geometry object that specifies the grid of the result image. If not defined the geometry of the input image will be used.
     * @param paddingValue Indicates the value that should be used if an out of input error occurs (and throwOnOutOfInputAreaError is false).
     * @param interpolatorType Indicates the type of interpolation strategy that should be used.
     * @pre input must be valid
     * @pre registration must be valid
     * @pre Dimensionality of the registration must match with the input imageinput must be valid
     * @remark Depending in the settings of throwOnOutOfInputAreaError and throwOnMappingError it may also throw
     * due to inconsistencies in the mapping process. See parameter description.
     * @result Pointer to the resulting mapped image.h*/
    MITKMATCHPOINTREGISTRATION_EXPORT Image::Pointer StitchImages(std::vector<Image::ConstPointer> inputs,
      std::vector<::map::core::RegistrationBase::ConstPointer> registrations,
      const BaseGeometry* resultGeometry,
      const double& paddingValue = 0,
      mitk::ImageMappingInterpolator::Type interpolatorType = mitk::ImageMappingInterpolator::Linear);

    MITKMATCHPOINTREGISTRATION_EXPORT Image::Pointer StitchImages(std::vector<Image::ConstPointer> inputs,
      std::vector<MAPRegistrationWrapper::ConstPointer> registrations,
      const BaseGeometry* resultGeometry,
      const double& paddingValue = 0,
      mitk::ImageMappingInterpolator::Type interpolatorType = mitk::ImageMappingInterpolator::Linear);

    MITKMATCHPOINTREGISTRATION_EXPORT Image::Pointer StitchImages(std::vector<Image::ConstPointer> inputs,
      const BaseGeometry* resultGeometry,
      const double& paddingValue = 0,
      mitk::ImageMappingInterpolator::Type interpolatorType = mitk::ImageMappingInterpolator::Linear);

}

#endif
