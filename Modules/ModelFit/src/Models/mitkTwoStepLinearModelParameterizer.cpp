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
  initialParameters[mitk:: TwoStepLinearModel::POSITION_PARAMETER_y0] = 0.0;
  initialParameters[mitk:: TwoStepLinearModel::POSITION_PARAMETER_x0] = 50;
  initialParameters[mitk:: TwoStepLinearModel::POSITION_PARAMETER_b0] = 1.0;
  initialParameters[mitk:: TwoStepLinearModel::POSITION_PARAMETER_b1] = -1.0;

  return initialParameters;
};


