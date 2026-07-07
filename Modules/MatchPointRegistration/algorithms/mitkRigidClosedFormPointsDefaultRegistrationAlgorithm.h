/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRigidClosedFormPointsDefaultRegistrationAlgorithm_h
#define mitkRigidClosedFormPointsDefaultRegistrationAlgorithm_h

#include <mapDiscreteElements.h>
#include <mapITKRigid3DClosedFormRegistrationAlgorithmTemplate.h>
#include <mapConfigure.h>

#include <mitkRigidClosedFormPointsDefaultRegistrationAlgorithm_ProfileResource.h>

namespace mitk
{
  /**
   * \brief Default rigid closed-form point-based registration algorithm.
   *
   * Type alias for the MatchPoint ITK rigid 3D closed-form registration
   * algorithm operating on point sets with a MITK-specific UID policy.
   *
   * \tparam TPointSetType The ITK point set type to register.
   */
  template <typename TPointSetType>
  using RigidClosedFormPointsDefaultRegistrationAlgorithm = typename map::algorithm::boxed::ITKRigid3DClosedFormRegistrationAlgorithmTemplate<TPointSetType, ::map::algorithm::mitkRigidClosedFormPointsDefaultRegistrationAlgorithmUIDPolicy>::Type;
}

#endif
