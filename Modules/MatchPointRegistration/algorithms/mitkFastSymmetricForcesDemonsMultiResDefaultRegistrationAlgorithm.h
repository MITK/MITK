/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFastSymmetricForcesDemonsMultiResDefaultRegistrationAlgorithm_h
#define mitkFastSymmetricForcesDemonsMultiResDefaultRegistrationAlgorithm_h

#include <mapDiscreteElements.h>
#include <mapITKFastSymmetricForcesDemonsMultiResRegistrationAlgorithm.h>
#include <mapConfigure.h>

#include <mitkFastSymmetricForcesDemonsMultiResDefaultRegistrationAlgorithm_ProfileResource.h>

namespace mitk
{
  /**
   * \brief Default multi-resolution fast symmetric forces demons registration algorithm.
   *
   * Type alias for the MatchPoint ITK fast symmetric forces demons multi-resolution
   * registration algorithm with a MITK-specific UID policy.
   *
   * \tparam TImageType The ITK image type to register.
   */
  template <typename TImageType>
  using FastSymmetricForcesDemonsMultiResDefaultRegistrationAlgorithm = map::algorithm::boxed::ITKFastSymmetricForcesDemonsMultiResRegistrationAlgorithm<TImageType, map::algorithm::mitkFastSymmetricForcesDemonsMultiResDefaultRegistrationAlgorithmUIDPolicy>;
}

#endif
