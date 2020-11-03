/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkStandardToftsModelParameterizer.h"

mitk::StandardToftsModelParameterizer::ParametersType
mitk::StandardToftsModelParameterizer::GetDefaultInitialParameterization() const
{
  ParametersType initialParameters;
  initialParameters.SetSize(2);
  initialParameters[mitk::StandardToftsModel::POSITION_PARAMETER_Ktrans] = 15;
  initialParameters[mitk::StandardToftsModel::POSITION_PARAMETER_ve] = 0.5;

  return initialParameters;
};

mitk::StandardToftsModelParameterizer::StandardToftsModelParameterizer()
{
};

mitk::StandardToftsModelParameterizer::~StandardToftsModelParameterizer()
{
};
