/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mapDeploymentDLLHelper.h"
#include "mapContinuousElements.h"
#include "mapConfigure.h"

#include "mitkRigidICPDefaultRegistrationAlgorithm.h"

typedef map::core::continuous::Elements<3>::InternalPointSetType PointSetType;

mapDeployAlgorithmMacro(mitk::RigidICPDefaultRegistrationAlgorithm<PointSetType>);
