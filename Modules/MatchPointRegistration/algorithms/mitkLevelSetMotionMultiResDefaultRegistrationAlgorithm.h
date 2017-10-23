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

#ifndef mitkLevelSetMotionMultiResDefaultRegistrationAlgorithm_h
#define mitkLevelSetMotionMultiResDefaultRegistrationAlgorithm_h

#include "mapDiscreteElements.h"
#include "mapITKLevelSetMotionMultiResRegistrationAlgorithm.h"
#include "mapConfigure.h"

#include "mitkLevelSetMotionMultiResDefaultRegistrationAlgorithm_ProfileResource.h"

namespace mitk
{
  template <typename TImageType>
  using LevelSetMotionMultiResDefaultRegistrationAlgorithm = map::algorithm::boxed::ITKLevelSetMotionMultiResRegistrationAlgorithm<TImageType, map::algorithm::mitkLevelSetMotionMultiResDefaultRegistrationAlgorithmUIDPolicy>;
}

#endif
