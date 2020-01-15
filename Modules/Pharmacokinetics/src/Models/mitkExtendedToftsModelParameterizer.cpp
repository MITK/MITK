/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkExtendedToftsModelParameterizer.h"

mitk::ExtendedToftsModelParameterizer::ParametersType
mitk::ExtendedToftsModelParameterizer::GetDefaultInitialParameterization() const
{
  ParametersType initialParameters;
  initialParameters.SetSize(3);
  initialParameters[mitk::ExtendedToftsModel::POSITION_PARAMETER_Ktrans] = 15;
  initialParameters[mitk::ExtendedToftsModel::POSITION_PARAMETER_ve] = 0.5;
  initialParameters[mitk::ExtendedToftsModel::POSITION_PARAMETER_vp] = 0.05;

  return initialParameters;
};

mitk::ExtendedToftsModelParameterizer::ExtendedToftsModelParameterizer()
{
};

mitk::ExtendedToftsModelParameterizer::~ExtendedToftsModelParameterizer()
{
};
