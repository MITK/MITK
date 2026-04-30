/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRigidICPDefaultRegistrationAlgorithm_h
#define mitkRigidICPDefaultRegistrationAlgorithm_h

#include <mapDiscreteElements.h>
#include <mapITKRigid3DICPRegistrationAlgorithmTemplate.h>
#include <mapConfigure.h>

#include <mitkRigidICPDefaultRegistrationAlgorithm_ProfileResource.h>

namespace mitk
{
  /**
   * \brief Default rigid ICP (Iterative Closest Point) registration algorithm.
   *
   * Type alias for the MatchPoint ITK rigid 3D ICP registration algorithm
   * operating on point sets with a MITK-specific UID policy.
   *
   * \tparam TPointSetType The ITK point set type to register.
   */
  template <typename TPointSetType>
  using RigidICPDefaultRegistrationAlgorithm = map::algorithm::boxed::ITKRigid3DICPRegistrationAlgorithm<TPointSetType, TPointSetType, ::map::algorithm::mitkRigidICPDefaultRegistrationAlgorithmUIDPolicy>;

}

#endif
