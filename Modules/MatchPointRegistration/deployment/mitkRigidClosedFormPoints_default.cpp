/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mapDeploymentDLLHelper.h"
#include "mapContinuousElements.h"
#include "mapITKRigid3DClosedFormRegistrationAlgorithmTemplate.h"
#include "mapConfigure.h"

#include "mitkRigidClosedFormPointsDefaultRegistrationAlgorithm.h"

typedef map::core::continuous::Elements<3>::InternalPointSetType PointSetType;

mapDeployAlgorithmMacro(mitk::RigidClosedFormPointsDefaultRegistrationAlgorithm<PointSetType>);
