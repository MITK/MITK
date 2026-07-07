/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiModalRigidHeadNeckRegistrationAlgorithm_h
#define mitkMultiModalRigidHeadNeckRegistrationAlgorithm_h

#include <mapDiscreteElements.h>
#include <mapITKRigid3DMattesMIHeadNeckRegistrationAlgorithmTemplate.h>
#include <mapConfigure.h>

#include <mitkMultiModalRigidHeadNeckRegistrationAlgorithm_ProfileResource.h>

namespace mitk
{
  /**
   * \brief Rigid head-neck registration algorithm for multi-modal images.
   *
   * Type alias for the MatchPoint rigid 3D head-neck registration algorithm
   * using Mattes mutual information with a MITK-specific UID policy.
   *
   * \tparam TImageType The ITK image type to register.
   */
  template <class TImageType>
  using MultiModalRigidHeadNeckRegistrationAlgorithm = typename map::algorithm::boxed::MultiModalRigidHeadNeckRegistrationAlgorithm<TImageType, ::map::algorithm::mitkMultiModalRigidHeadNeckRegistrationAlgorithmUIDPolicy>;
}

#endif
