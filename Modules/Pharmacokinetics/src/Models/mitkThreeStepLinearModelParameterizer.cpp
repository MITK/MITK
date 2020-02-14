/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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


