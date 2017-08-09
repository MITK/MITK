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

#ifndef _MITK_MATCHPOINTPROPERTY_TAGS__H_
#define _MITK_MATCHPOINTPROPERTY_TAGS__H_

// MITK
#include "MitkMatchPointRegistrationExports.h"

namespace mitk
{
  /**UID of the algorithm that was used to determin a registration.*/
  const char* const Prop_RegAlgUsed = "matchpoint.Registration.Algorithm.UID";
  /**UID(s) of the data object(s) used as target for determining the registration.*/
  const char* const Prop_RegAlgTargetData = "matchpoint.Registration.Algorithm.UsedData.target";
  /**UID(s) of the data object(s) used as moving objects for determining the registration.*/
  const char* const Prop_RegAlgMovingData = "matchpoint.Registration.Algorithm.UsedData.moving";
  /**UID of the registration instance.*/
  const char* const Prop_RegUID = "matchpoint.Registration.UID";
  /**Input "section" that specifies what wwas mapped.*/
  const char* const Prop_MappingInput = "matchpoint.Mapping.Input";
  /**UID of the data object that was mapped (so the source) by the specified registration to generate the current instance.*/
  const char* const Prop_MappingInputData = "matchpoint.Mapping.Input.Data";
  /**Type of the interpolation strategy that was used to map the object. If not set, no interpolation was needed for mapping.*/
  const char* const Prop_MappingInterpolator = "matchpoint.Mapping.Interpolator";
  /**Indicates that the data was not mapped (in termes of resampled), but "just" the geometry was refined.*/
  const char* const Prop_MappingRefinedGeometry = "matchpoint.Mapping.RefinedGeometry";
  /**MatchPoint UID to uniquely identify an data object.*/
  const char* const Prop_UID = "data.UID";
  /**MatchPoint UID to uniquely identify an node.*/
  const char* const nodeProp_UID = "matchpoint.UID";
}


#endif


