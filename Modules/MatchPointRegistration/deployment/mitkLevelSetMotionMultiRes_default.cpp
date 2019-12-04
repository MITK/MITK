/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mapDeploymentDLLHelper.h"
#include "mapDiscreteElements.h"
#include "mapConfigure.h"

#include "mitkLevelSetMotionMultiResDefaultRegistrationAlgorithm.h"
typedef map::core::discrete::Elements<3>::InternalImageType ImageType;

mapDeployAlgorithmMacro(mitk::LevelSetMotionMultiResDefaultRegistrationAlgorithm<ImageType>);
