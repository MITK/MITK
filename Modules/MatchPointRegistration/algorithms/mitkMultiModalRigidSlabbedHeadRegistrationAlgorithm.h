/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiModalRigidSlabbedHeadRegistrationAlgorithm_h
#define mitkMultiModalRigidSlabbedHeadRegistrationAlgorithm_h

#include "mapDiscreteElements.h"
#include "mapITKRigid3DMattesMISlabbedHeadRegistrationAlgorithmTemplate.h"
#include "mapConfigure.h"

#include "mitkMultiModalRigidSlabbedHeadRegistrationAlgorithm_ProfileResource.h"

namespace mitk
{
  template <class TImageType>
  using MultiModalRigidSlabbedHeadRegistrationAlgorithm = typename map::algorithm::boxed::MultiModalRigidSlabbedHeadRegistrationAlgorithm<TImageType, ::map::algorithm::mitkMultiModalRigidSlabbedHeadRegistrationAlgorithmUIDPolicy>;
}

#endif