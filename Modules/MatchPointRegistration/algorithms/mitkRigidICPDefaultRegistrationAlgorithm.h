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

#ifndef mitkRigidICPDefaultRegistrationAlgorithm_h
#define mitkRigidICPDefaultRegistrationAlgorithm_h

#include "mapDiscreteElements.h"
#include "mapITKRigid3DICPRegistrationAlgorithmTemplate.h"
#include "mapConfigure.h"

#include "mitkRigidICPDefaultRegistrationAlgorithm_ProfileResource.h"

namespace mitk
{
  template <typename TPointSetType>
  using RigidICPDefaultRegistrationAlgorithm = map::algorithm::boxed::ITKRigid3DICPRegistrationAlgorithm<TPointSetType, TPointSetType, ::map::algorithm::mitkRigidICPDefaultRegistrationAlgorithmUIDPolicy>;

}

#endif
