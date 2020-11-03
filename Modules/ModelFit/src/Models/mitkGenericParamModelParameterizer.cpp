/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkGenericParamModelParameterizer.h"

mitk::ModelBase::FunctionStringType mitk::GenericParamModelParameterizer::GetFunctionString() const
{
  return m_FunctionString;
};

mitk::GenericParamModelParameterizer::ModelBasePointer
  mitk::GenericParamModelParameterizer::GenerateParameterizedModel(const IndexType& currentPosition) const
{
  ModelPointer newModel = dynamic_cast<ModelType*>(Superclass::GenerateParameterizedModel(
                            currentPosition).GetPointer());
  newModel->SetFunctionString(m_FunctionString);
  return newModel.GetPointer();
};

mitk::GenericParamModelParameterizer::StaticParameterMapType
  mitk::GenericParamModelParameterizer::GetGlobalStaticParameters() const
{
  StaticParameterMapType result;
  StaticParameterValuesType values;
  values.push_back(m_NumberOfParameters);
  result.insert(std::make_pair(ModelType::NAME_STATIC_PARAMETER_number, values));
  return result;
};

mitk::GenericParamModelParameterizer::ParametersType
  mitk::GenericParamModelParameterizer::GetDefaultInitialParameterization() const
{
  ParametersType initialParameters;
  initialParameters.SetSize(m_NumberOfParameters);
  initialParameters.Fill(1.0);
  return initialParameters;
};

mitk::GenericParamModelParameterizer::GenericParamModelParameterizer():m_NumberOfParameters(1)
{};

mitk::GenericParamModelParameterizer::~GenericParamModelParameterizer()
= default;
