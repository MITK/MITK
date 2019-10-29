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
