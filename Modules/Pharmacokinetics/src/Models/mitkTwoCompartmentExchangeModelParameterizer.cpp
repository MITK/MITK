/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTwoCompartmentExchangeModelParameterizer.h"

mitk::TwoCompartmentExchangeModelParameterizer::ParametersType
mitk::TwoCompartmentExchangeModelParameterizer::GetDefaultInitialParameterization() const
{
  ParametersType initialParameters;
  initialParameters.SetSize(4);
  initialParameters[mitk::TwoCompartmentExchangeModel::POSITION_PARAMETER_PS] = 5.0;
  initialParameters[mitk::TwoCompartmentExchangeModel::POSITION_PARAMETER_F] = 20.0;
  initialParameters[mitk::TwoCompartmentExchangeModel::POSITION_PARAMETER_ve] = 0.1;
  initialParameters[mitk::TwoCompartmentExchangeModel::POSITION_PARAMETER_vp] = 0.04;

  return initialParameters;
};

mitk::TwoCompartmentExchangeModelParameterizer::TwoCompartmentExchangeModelParameterizer()
{
};

mitk::TwoCompartmentExchangeModelParameterizer::~TwoCompartmentExchangeModelParameterizer()
{
};
