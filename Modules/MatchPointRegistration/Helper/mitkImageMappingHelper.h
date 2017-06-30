/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef MITK_IMAGE_MAPPING_HELPER_H
#define MITK_IMAGE_MAPPING_HELPER_H

#include "mapImageMappingTask.h"
#include "mapRegistrationBase.h"
#include "mitkImage.h"
#include "mitkGeometry3D.h"

#include "mitkMAPRegistrationWrapper.h"

#include "MitkMatchPointRegistrationExports.h"

namespace mitk
{
  struct ImageMappingInterpolator
  {
    enum Type
    {
      UserDefined = 0, //< User may specify the interpolator to use
      NearestNeighbor = 1, //< use nearest neighbor
      Linear = 2, //< use linear
      BSpline_3 = 3, //< use a 3rd order spline
      WSinc_Hamming = 4, //< use a wsinc with hamming window
      WSinc_Welch = 5 //< use a wsinc with welch window
    };
  };

  namespace ImageMappingHelper
  {
    typedef ::map::core::RegistrationBase RegistrationType;
    typedef ::mitk::MAPRegistrationWrapper MITKRegistrationType;

    typedef ::mitk::BaseGeometry ResultImageGeometryType;
    typedef ::mitk::Image InputImageType;
    typedef ::mitk::Image ResultImageType;

    /**Helper that maps a given input image
     * @param input Image that should be mapped.
     * @param registration Pointer to the registration instance that should be used for mapping
     * @param throwOnOutOfInputAreaError Indicates if mapping should fail with an exception (true), if the input image does not cover the whole requested region to be mapped into the result image.
     * @param paddingValue Indicates the value that should be used if an out of input error occurs (and throwOnOutOfInputAreaError is false).
     * @param resultGeometry Pointer to the Geometry object that specifies the grid of the result image. If not defined the geometry of the input image will be used.
     * @param throwOnMappingError Indicates if mapping should fail with an exception (true), if the registration does not cover/support the whole requested region for mapping into the result image.
     * @param errorValue Indicates the value that should be used if an mapping error occurs (and throwOnMappingError is false).
     * @param interpolatorType Indicates the type of interpolation strategy that should be used.
     * @pre input must be valid
     * @pre registration must be valid
     * @pre Dimensionality of the registration must match with the input imageinput must be valid
     * @remark Depending in the settings of throwOnOutOfInputAreaError and throwOnMappingError it may also throw
     * due to inconsistencies in the mapping process. See parameter description.
     * @result Pointer to the resulting mapped image.h*/
    MITKMATCHPOINTREGISTRATION_EXPORT ResultImageType::Pointer map(const InputImageType* input, const RegistrationType* registration,
      bool throwOnOutOfInputAreaError = false, const double& paddingValue = 0,
      const ResultImageGeometryType* resultGeometry = nullptr,
      bool throwOnMappingError = true, const double& errorValue = 0, mitk::ImageMappingInterpolator::Type interpolatorType = mitk::ImageMappingInterpolator::Linear);

    /**Helper that maps a given input image.
     * @overload
     * @param input Image that should be mapped.
     * @param registration Pointer to the registration instance that should be used for mapping
     * @param throwOnOutOfInputAreaError Indicates if mapping should fail with an exception (true), if the input image does not cover the whole requested region to be mapped into the result image.
     * @param paddingValue Indicates the value that should be used if an out of input error occurs (and throwOnOutOfInputAreaError is false).
     * @param resultGeometry Pointer to the Geometry object that specifies the grid of the result image. If not defined the geometry of the input image will be used.
     * @param throwOnMappingError Indicates if mapping should fail with an exception (true), if the registration does not cover/support the whole requested region for mapping into the result image.
     * @param errorValue Indicates the value that should be used if an mapping error occurs (and throwOnMappingError is false).
     * @param interpolatorType Indicates the type of interpolation strategy that should be used.
     * @pre input must be valid
     * @pre registration must be valid
     * @pre Dimensionality of the registration must match with the input imageinput must be valid
     * @remark Depending in the settings of throwOnOutOfInputAreaError and throwOnMappingError it may also throw
     * due to inconsistencies in the mapping process. See parameter description.
     * @result Pointer to the resulting mapped image.h*/
    MITKMATCHPOINTREGISTRATION_EXPORT ResultImageType::Pointer map(const InputImageType* input, const MITKRegistrationType* registration,
      bool throwOnOutOfInputAreaError = false, const double& paddingValue = 0,
      const ResultImageGeometryType* resultGeometry = nullptr,
      bool throwOnMappingError = true, const double& errorValue = 0, mitk::ImageMappingInterpolator::Type interpolatorType = mitk::ImageMappingInterpolator::Linear);

    /**Method clones the input image and applies the registration by applying it to the Geometry3D of the image.
    Thus this method only produces a result if the passed registration has an direct mapping kernel that
    can be converted into an affine matrix transformation.
    @pre input must point to a valid instance
    @pre registration must point to a valid instance
    @pre registration must be decomposable into rotation matrix and offset or throwOnError must be false.
    @result Pointer to the result image with refined geometry. May be null if the result cannot be
    generated (e.g. the registration cannot be transformed in a affine matrix transformation)*/
    MITKMATCHPOINTREGISTRATION_EXPORT ResultImageType::Pointer refineGeometry(const InputImageType* input, const RegistrationType* registration, bool throwOnError = true);
    /**@overload*/
    MITKMATCHPOINTREGISTRATION_EXPORT ResultImageType::Pointer refineGeometry(const InputImageType* input, const MITKRegistrationType* registration, bool throwOnError = true);

    /**Method allows to pre checks if a registration could be used with refineGeometry. If this method returns true,
    * the registration can be used to refine the geometry of images.*/
    MITKMATCHPOINTREGISTRATION_EXPORT bool canRefineGeometry(const RegistrationType* registration);
    /**@overload*/
    MITKMATCHPOINTREGISTRATION_EXPORT bool canRefineGeometry(const MITKRegistrationType* registration);

  }

}

#endif
