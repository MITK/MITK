/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTwoStepLinearModelParameterizer.h"

mitk::TwoStepLinearModelParameterizer::ParametersType
mitk::TwoStepLinearModelParameterizer::GetDefaultInitialParameterization() const
{
  ParametersType initialParameters;
  initialParameters.SetSize(4);
  initialParameters[mitk:: TwoStepLinearModel::POSITION_PARAMETER_y1] = 0.0;
  initialParameters[mitk:: TwoStepLinearModel::POSITION_PARAMETER_t] = 50;
  initialParameters[mitk:: TwoStepLinearModel::POSITION_PARAMETER_a1] = 1.0;
  initialParameters[mitk:: TwoStepLinearModel::POSITION_PARAMETER_a2] = -1.0;

  return initialParameters;
};


