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

#ifndef mitkRigidClosedFormPointsDefaultRegistrationAlgorithm_h
#define mitkRigidClosedFormPointsDefaultRegistrationAlgorithm_h

#include "mapDiscreteElements.h"
#include "mapITKRigid3DClosedFormRegistrationAlgorithmTemplate.h"
#include "mapConfigure.h"

#include "mitkRigidClosedFormPointsDefaultRegistrationAlgorithm_ProfileResource.h"

namespace mitk
{
  template <typename TPointSetType>
  using RigidClosedFormPointsDefaultRegistrationAlgorithm = typename map::algorithm::boxed::ITKRigid3DClosedFormRegistrationAlgorithmTemplate<TPointSetType, ::map::algorithm::mitkRigidClosedFormPointsDefaultRegistrationAlgorithmUIDPolicy>::Type;
}

#endif
