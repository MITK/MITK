/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkExponentialDecayModel.h"
#include "mitkNumericConstants.h"

const std::string mitk::ExponentialDecayModel::NAME_PARAMETER_y0 = "y-intercept";
const std::string mitk::ExponentialDecayModel::NAME_PARAMETER_lambda = "lambda";

const unsigned int mitk::ExponentialDecayModel::NUMBER_OF_PARAMETERS = 2;

const std::string mitk::ExponentialDecayModel::UNIT_PARAMETER_y0 = "[unit of y]";
const std::string mitk::ExponentialDecayModel::UNIT_PARAMETER_lambda = "[unit of x]";

const unsigned int mitk::ExponentialDecayModel::POSITION_PARAMETER_y0 = 0;
const unsigned int mitk::ExponentialDecayModel::POSITION_PARAMETER_lambda = 1;

const std::string mitk::ExponentialDecayModel::NAME_DERIVED_PARAMETER_k = "rate";

const unsigned int mitk::ExponentialDecayModel::NUMBER_OF_DERIVED_PARAMETERS = 1;

const std::string mitk::ExponentialDecayModel::UNIT_DERIVED_PARAMETER_k = "1/[unit of x]";

const unsigned int mitk::ExponentialDecayModel::NUMBER_OF_STATIC_PARAMETERS = 0;

const std::string mitk::ExponentialDecayModel::MODEL_DISPLAY_NAME = "Exponential Decay Model";

const std::string mitk::ExponentialDecayModel::MODEL_TYPE = "Generic";

const std::string mitk::ExponentialDecayModel::FUNCTION_STRING = "y(x) = y-intercept * exp(-x/lambda)";

const std::string mitk::ExponentialDecayModel::X_NAME = "x";

const std::string mitk::ExponentialDecayModel::X_AXIS_NAME = "X";

const std::string mitk::ExponentialDecayModel::X_AXIS_UNIT = "unit of x";

const std::string mitk::ExponentialDecayModel::Y_AXIS_NAME = "Y";

const std::string mitk::ExponentialDecayModel::Y_AXIS_UNIT = "unit of y";


std::string mitk::ExponentialDecayModel::GetModelDisplayName() const
{
  return MODEL_DISPLAY_NAME;
};

std::string mitk::ExponentialDecayModel::GetModelType() const
{
  return MODEL_TYPE;
};

mitk::ExponentialDecayModel::FunctionStringType mitk::ExponentialDecayModel::GetFunctionString() const
{
  return FUNCTION_STRING;
};

std::string mitk::ExponentialDecayModel::GetXName() const
{
  return X_NAME;
};

std::string mitk::ExponentialDecayModel::GetXAxisName() const
{
  return X_AXIS_NAME;
};

std::string mitk::ExponentialDecayModel::GetXAxisUnit() const
{
  return X_AXIS_UNIT;
}

std::string mitk::ExponentialDecayModel::GetYAxisName() const
{
  return Y_AXIS_NAME;
};

std::string mitk::ExponentialDecayModel::GetYAxisUnit() const
{
  return Y_AXIS_UNIT;
}

mitk::ExponentialDecayModel::ParameterNamesType
mitk::ExponentialDecayModel::GetParameterNames() const
{
  ParameterNamesType result;
  result.push_back(NAME_PARAMETER_y0);
  result.push_back(NAME_PARAMETER_lambda);
  return result;
};

mitk::ExponentialDecayModel::ParamterUnitMapType mitk::ExponentialDecayModel::GetParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair(NAME_PARAMETER_y0, UNIT_PARAMETER_y0));
  result.insert(std::make_pair(NAME_PARAMETER_lambda, UNIT_PARAMETER_lambda));

  return result;
}

mitk::ExponentialDecayModel::ParametersSizeType
mitk::ExponentialDecayModel::GetNumberOfParameters() const
{
  return NUMBER_OF_PARAMETERS;
};

mitk::ExponentialDecayModel::ParameterNamesType
mitk::ExponentialDecayModel::GetDerivedParameterNames() const
{
  ParameterNamesType result;
  result.push_back(NAME_DERIVED_PARAMETER_k);
  return result;
};

mitk::ExponentialDecayModel::ParametersSizeType
mitk::ExponentialDecayModel::GetNumberOfDerivedParameters() const
{
  return NUMBER_OF_DERIVED_PARAMETERS;
};

mitk::ExponentialDecayModel::ParamterUnitMapType mitk::ExponentialDecayModel::GetDerivedParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair(NAME_DERIVED_PARAMETER_k, UNIT_DERIVED_PARAMETER_k));

  return result;
};

mitk::ExponentialDecayModel::ModelResultType
mitk::ExponentialDecayModel::ComputeModelfunction(const ParametersType& parameters) const
{
  double     y0 = parameters[POSITION_PARAMETER_y0];
  double     lambda = parameters[POSITION_PARAMETER_lambda];

  ModelResultType signal(m_TimeGrid.GetSize());

  ModelResultType::iterator signalPos = signal.begin();

  for (const auto& gridPos : m_TimeGrid)
  {
    *signalPos = y0 * exp(-1.0 * gridPos/ lambda);
    ++signalPos;
  }

  return signal;
};

mitk::ExponentialDecayModel::ParameterNamesType mitk::ExponentialDecayModel::GetStaticParameterNames() const
{
  ParameterNamesType result;

  return result;
}

mitk::ExponentialDecayModel::ParametersSizeType  mitk::ExponentialDecayModel::GetNumberOfStaticParameters() const
{
  return NUMBER_OF_STATIC_PARAMETERS;
}

void mitk::ExponentialDecayModel::SetStaticParameter(const ParameterNameType& /*name*/,
    const StaticParameterValuesType& /*values*/)
{
  //do nothing
};

mitk::ExponentialDecayModel::StaticParameterValuesType mitk::ExponentialDecayModel::GetStaticParameterValue(
  const ParameterNameType& /*name*/) const
{
  StaticParameterValuesType result;

  //do nothing

  return result;
};


mitk::ModelBase::DerivedParameterMapType mitk::ExponentialDecayModel::ComputeDerivedParameters(
  const mitk::ModelBase::ParametersType &parameters) const
{
  DerivedParameterMapType result;

  //Model Parameters
  double     lambda = parameters[POSITION_PARAMETER_lambda];

  double k = 1.0 / (lambda + mitk::eps);
  result.insert(std::make_pair(NAME_DERIVED_PARAMETER_k, k));
  return result;
};

itk::LightObject::Pointer mitk::ExponentialDecayModel::InternalClone() const
{
  ExponentialDecayModel::Pointer newClone = ExponentialDecayModel::New();

  newClone->SetTimeGrid(this->m_TimeGrid);

  return newClone.GetPointer();
};
