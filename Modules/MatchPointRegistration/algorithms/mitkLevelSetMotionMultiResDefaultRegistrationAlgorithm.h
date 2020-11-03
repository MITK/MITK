/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
