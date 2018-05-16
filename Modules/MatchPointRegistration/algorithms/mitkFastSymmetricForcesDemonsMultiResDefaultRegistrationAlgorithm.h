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

#ifndef mitkFastSymmetricForcesDemonsMultiResDefaultRegistrationAlgorithm_h
#define mitkFastSymmetricForcesDemonsMultiResDefaultRegistrationAlgorithm_h

#include "mapDiscreteElements.h"
#include "mapITKFastSymmetricForcesDemonsMultiResRegistrationAlgorithm.h"
#include "mapConfigure.h"

#include "mitkFastSymmetricForcesDemonsMultiResDefaultRegistrationAlgorithm_ProfileResource.h"

namespace mitk
{
  template <typename TImageType>
  using FastSymmetricForcesDemonsMultiResDefaultRegistrationAlgorithm = map::algorithm::boxed::ITKFastSymmetricForcesDemonsMultiResRegistrationAlgorithm<TImageType, map::algorithm::mitkFastSymmetricForcesDemonsMultiResDefaultRegistrationAlgorithmUIDPolicy>;
}

#endif
