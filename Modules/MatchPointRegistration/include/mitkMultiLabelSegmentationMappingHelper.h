/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkMultiLabelSegmentationMappingHelper_h
#define mitkMultiLabelSegmentationMappingHelper_h

#include "mapRegistrationBase.h"
#include "mitkLabelSetImage.h"
#include "mitkGeometry3D.h"

#include "mitkMAPRegistrationWrapper.h"

#include "MitkMatchPointRegistrationExports.h"

namespace mitk
{

  namespace MultiLabelSegmentationMappingHelper
  {
    typedef ::map::core::RegistrationBase RegistrationType;
    typedef ::mitk::MAPRegistrationWrapper MITKRegistrationType;

    typedef ::mitk::BaseGeometry ResultGeometryType;

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
     * @pre Dimensionality of the registration must match with the input image must be valid
     * @remark Depending in the settings of throwOnOutOfInputAreaError and throwOnMappingError it may also throw
     * due to inconsistencies in the mapping process. See parameter description.
     * @result Pointer to the resulting mapped image.h */
    MITKMATCHPOINTREGISTRATION_EXPORT MultiLabelSegmentation::Pointer map(const MultiLabelSegmentation* input, const RegistrationType* registration,
      bool throwOnOutOfInputAreaError = false, const ResultGeometryType* resultGeometry = nullptr,
      bool throwOnMappingError = true, const MultiLabelSegmentation::LabelValueType& errorValue = 0);

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
     * @pre Dimensionality of the registration must match with the input image must be valid
     * @remark Depending in the settings of throwOnOutOfInputAreaError and throwOnMappingError it may also throw
     * due to inconsistencies in the mapping process. See parameter description.
     * @result Pointer to the resulting mapped image.h*/
    MITKMATCHPOINTREGISTRATION_EXPORT MultiLabelSegmentation::Pointer map(const MultiLabelSegmentation* input, const MITKRegistrationType* registration,
      bool throwOnOutOfInputAreaError = false, const ResultGeometryType* resultGeometry = nullptr,
      bool throwOnMappingError = true, const MultiLabelSegmentation::LabelValueType& errorValue = 0);

  }

}

#endif
