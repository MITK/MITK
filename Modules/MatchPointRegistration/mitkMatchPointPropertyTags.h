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
const char* const nodeProp_RegAlgUsed = "matchpoint.Registration.Algorithm.UID";
const char* const nodeProp_RegAlgTargetData = "matchpoint.Registration.Algorithm.UsedData.target";
const char* const nodeProp_RegAlgMovingData = "matchpoint.Registration.Algorithm.UsedData.moving";
const char* const nodeProp_RegUID = "matchpoint.Registration.UID";
const char* const nodeProp_MappingInput = "matchpoint.Mapping.Input";
const char* const nodeProp_MappingInputData = "matchpoint.Mapping.Input.Data";
const char* const nodeProp_MappingInterpolator = "matchpoint.Mapping.Interpolator";
const char* const nodeProp_MappingRefinedGeometry = "matchpoint.Mapping.RefinedGeometry";
const char* const nodeProp_UID = "matchpoint.UID";

}


#endif


