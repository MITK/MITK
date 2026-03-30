/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMatchPointPropertyTags_h
#define mitkMatchPointPropertyTags_h

// MITK
#include <MitkMatchPointRegistrationExports.h>

/**
 * \brief Property tag constants used by the MatchPoint registration framework in MITK.
 *
 * These string constants define the property keys that are attached to DataNode and BaseData
 * objects to store MatchPoint-related metadata such as registration algorithm UIDs,
 * input data references, mapping parameters, and unique identifiers.
 *
 * \sa mitk::MAPRegistrationWrapper, mitk::MITKRegistrationHelper
 */
namespace mitk
{
  /** \brief Property key storing the UID of the algorithm that was used to determine a registration. */
  const char* const Prop_RegAlgUsed = "matchpoint.Registration.Algorithm.UID";
  /** \brief Property key storing the UID(s) of the data object(s) used as target for determining the registration. */
  const char* const Prop_RegAlgTargetData = "matchpoint.Registration.Algorithm.UsedData.target";
  /** \brief Property key storing the UID(s) of the data object(s) used as moving objects for determining the registration. */
  const char* const Prop_RegAlgMovingData = "matchpoint.Registration.Algorithm.UsedData.moving";
  /** \brief Property key storing the UID of the registration instance. */
  const char* const Prop_RegUID = "matchpoint.Registration.UID";
  /** \brief Property key for the input section that specifies what was mapped. */
  const char* const Prop_MappingInput = "matchpoint.Mapping.Input";
  /** \brief Property key storing the UID of the data object that was mapped (the source) by
   * the specified registration to generate the current instance. */
  const char* const Prop_MappingInputData = "matchpoint.Mapping.Input.Data";
  /** \brief Property key storing the type of the interpolation strategy that was used to map the object.
   *
   * If not set, no interpolation was needed for the mapping operation. */
  const char* const Prop_MappingInterpolator = "matchpoint.Mapping.Interpolator";
  /** \brief Property key indicating that the data was not mapped (resampled) but only had its geometry refined. */
  const char* const Prop_MappingRefinedGeometry = "matchpoint.Mapping.RefinedGeometry";
  /** \brief Property key for the MatchPoint UID used to uniquely identify a data object. */
  const char* const Prop_UID = "data.UID";
  /** \brief Property key for the MatchPoint UID used to uniquely identify a data node. */
  const char* const nodeProp_UID = "matchpoint.UID";
}


#endif
