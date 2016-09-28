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


#ifndef MITK_POINTSET_MAPPING_HELPER_H
#define MITK_POINTSET_MAPPING_HELPER_H

#include "mapRegistrationBase.h"
#include <mitkPointSet.h>

#include "mitkMAPRegistrationWrapper.h"

#include "MitkMatchPointRegistrationExports.h"

namespace mitk
{

  namespace PointSetMappingHelper
  {
    typedef ::map::core::RegistrationBase RegistrationType;
    typedef ::mitk::MAPRegistrationWrapper MITKRegistrationType;

    /**Helper that converts the data of an mitk point set into the default point set type of matchpoint.*/
    MITKMATCHPOINTREGISTRATION_EXPORT ::map::core::continuous::Elements<3>::InternalPointSetType::Pointer ConvertPointSetMITKtoMAP(const mitk::PointSet::DataType* mitkSet);

    /**Helper that maps a given input point set
     * @param input Point set that should be mapped.
     * @param registration Pointer to the registration instance that should be used for mapping
     * @param timeStep Indicates which time step of the point set should be mapped (the rest will just be copied). -1 (default) indicates that all time steps should be mapped.
     * @param throwOnMappingError Indicates if mapping should fail with an exception (true), if the registration does not cover/support the whole requested region for mapping into the result image.
     * if set to false, points that cause an mapping error will be transfered without mapping but get the passed errorPointValue as data to indicate unmappable points;
     * @param errorPointValue Indicates the point data that should be used if an mapping error occurs (and throwOnMappingError is false).
     * @pre input must be valid
     * @pre registration must be valid
     * @pre timeStep must be a valid time step of input or -1
     * @pre Dimensionality of the registration must match with the input imageinput must be valid
     * @remark Depending in the settings of throwOnMappingError it may also throw
     * due to inconsistencies in the mapping process. See parameter description.
     * @result Pointer to the resulting mapped point set*/
    MITKMATCHPOINTREGISTRATION_EXPORT ::mitk::PointSet::Pointer map(const ::mitk::PointSet* input, const RegistrationType* registration, int timeStep = -1,
      bool throwOnMappingError = true, const ::mitk::PointSet::PointDataType& errorPointValue = ::mitk::PointSet::PointDataType());

    /**Helper that maps a given input point set
     * @overload*/
    MITKMATCHPOINTREGISTRATION_EXPORT ::mitk::PointSet::Pointer map(const ::mitk::PointSet* input, const MITKRegistrationType* registration, int timeStep = -1,
      bool throwOnMappingError = true, const ::mitk::PointSet::PointDataType& errorPointValue = ::mitk::PointSet::PointDataType());
  }

}

#endif