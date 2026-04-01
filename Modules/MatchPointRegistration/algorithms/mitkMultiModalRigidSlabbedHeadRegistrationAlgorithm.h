/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiModalRigidSlabbedHeadRegistrationAlgorithm_h
#define mitkMultiModalRigidSlabbedHeadRegistrationAlgorithm_h

#include <mapDiscreteElements.h>
#include <mapITKRigid3DMattesMISlabbedHeadRegistrationAlgorithmTemplate.h>
#include <mapConfigure.h>

#include <mitkMultiModalRigidSlabbedHeadRegistrationAlgorithm_ProfileResource.h>

namespace mitk
{
  /**
   * \brief Rigid slabbed-head registration algorithm for multi-modal images.
   *
   * Type alias for the MatchPoint rigid 3D slabbed-head registration algorithm
   * using Mattes mutual information with a MITK-specific UID policy.
   *
   * \tparam TImageType The ITK image type to register.
   */
  template <class TImageType>
  using MultiModalRigidSlabbedHeadRegistrationAlgorithm = typename map::algorithm::boxed::MultiModalRigidSlabbedHeadRegistrationAlgorithm<TImageType, ::map::algorithm::mitkMultiModalRigidSlabbedHeadRegistrationAlgorithmUIDPolicy>;
}

#endif
