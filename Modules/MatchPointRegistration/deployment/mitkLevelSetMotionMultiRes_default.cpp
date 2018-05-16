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

#include "mapDeploymentDLLHelper.h"
#include "mapDiscreteElements.h"
#include "mapConfigure.h"

#include "mitkLevelSetMotionMultiResDefaultRegistrationAlgorithm.h"
typedef map::core::discrete::Elements<3>::InternalImageType ImageType;

mapDeployAlgorithmMacro(mitk::LevelSetMotionMultiResDefaultRegistrationAlgorithm<ImageType>);
