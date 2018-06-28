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

#include "mitkNumericTwoCompartmentExchangeModelParameterizer.h"

mitk::NumericTwoCompartmentExchangeModelParameterizer::ParametersType
mitk::NumericTwoCompartmentExchangeModelParameterizer::GetDefaultInitialParameterization() const
{
  ParametersType initialParameters;
  initialParameters.SetSize(4);
  initialParameters[mitk::NumericTwoCompartmentExchangeModel::POSITION_PARAMETER_PS] = 5.0;
  initialParameters[mitk::NumericTwoCompartmentExchangeModel::POSITION_PARAMETER_F] = 20.0;
  initialParameters[mitk::NumericTwoCompartmentExchangeModel::POSITION_PARAMETER_ve] = 0.1;
  initialParameters[mitk::NumericTwoCompartmentExchangeModel::POSITION_PARAMETER_vp] = 0.04;

  return initialParameters;
};

mitk::NumericTwoCompartmentExchangeModelParameterizer::NumericTwoCompartmentExchangeModelParameterizer()
{
};

mitk::NumericTwoCompartmentExchangeModelParameterizer::~NumericTwoCompartmentExchangeModelParameterizer()
{
};

mitk::NumericTwoCompartmentExchangeModelParameterizer::StaticParameterMapType mitk::NumericTwoCompartmentExchangeModelParameterizer::GetGlobalStaticParameters() const
{
  StaticParameterMapType result;
  StaticParameterValuesType valuesAIF = mitk::convertArrayToParameter(this->m_AIF);
  StaticParameterValuesType valuesAIFGrid = mitk::convertArrayToParameter(this->m_AIFTimeGrid);
  StaticParameterValuesType values;
  values.push_back(m_ODEINTStepSize);

  result.insert(std::make_pair(ModelType::NAME_STATIC_PARAMETER_AIF, valuesAIF));
  result.insert(std::make_pair(ModelType::NAME_STATIC_PARAMETER_AIFTimeGrid, valuesAIFGrid));
  result.insert(std::make_pair(ModelType::NAME_STATIC_PARAMETER_ODEINTStepSize, values));

  return result;
};
