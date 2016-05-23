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
#include "mapITKLevelSetMotionMultiResRegistrationAlgorithm.h"
#include "mapConfigure.h"

#include "MITK_LevelSetMotion_MultiRes_default_ProfileResource.h"
typedef map::core::discrete::Elements<3>::InternalImageType ImageType;
typedef map::algorithm::boxed::ITKLevelSetMotionMultiResRegistrationAlgorithm<ImageType, map::algorithm::MITK_LevelSetMotion_MultiRes_defaultUIDPolicy>
AlgorithmType;

mapDeployAlgorithmMacro(AlgorithmType);
