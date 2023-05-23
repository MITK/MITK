/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiModalRigidHeadNeckRegistrationAlgorithm_h
#define mitkMultiModalRigidHeadNeckRegistrationAlgorithm_h

#include "mapDiscreteElements.h"
#include "mapITKRigid3DMattesMIHeadNeckRegistrationAlgorithmTemplate.h"
#include "mapConfigure.h"

#include "mitkMultiModalRigidHeadNeckRegistrationAlgorithm_ProfileResource.h"

namespace mitk
{
  template <class TImageType>
  using MultiModalRigidHeadNeckRegistrationAlgorithm = typename map::algorithm::boxed::MultiModalRigidHeadNeckRegistrationAlgorithm<TImageType, ::map::algorithm::mitkMultiModalRigidHeadNeckRegistrationAlgorithmUIDPolicy>;
}

#endif