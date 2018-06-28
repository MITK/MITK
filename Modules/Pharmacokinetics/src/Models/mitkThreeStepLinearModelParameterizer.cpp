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

#include "mitkThreeStepLinearModelParameterizer.h"

mitk::ThreeStepLinearModelParameterizer::ParametersType
mitk::ThreeStepLinearModelParameterizer::GetDefaultInitialParameterization() const
{
  ParametersType initialParameters;
  initialParameters.SetSize(5);
  initialParameters[mitk:: ThreeStepLinearModel::POSITION_PARAMETER_S0] = 0.0;
  initialParameters[mitk:: ThreeStepLinearModel::POSITION_PARAMETER_t1] = 50;
  initialParameters[mitk:: ThreeStepLinearModel::POSITION_PARAMETER_t2] = 100;
  initialParameters[mitk:: ThreeStepLinearModel::POSITION_PARAMETER_a1] = 1.0;
  initialParameters[mitk:: ThreeStepLinearModel::POSITION_PARAMETER_a2] = -1.0;

  return initialParameters;
};


