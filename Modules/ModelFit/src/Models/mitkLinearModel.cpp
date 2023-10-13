/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkLinearModel.h"

const std::string mitk::LinearModel::NAME_PARAMETER_b = "slope";
const std::string mitk::LinearModel::NAME_PARAMETER_y0 = "y-intercept";

const unsigned int mitk::LinearModel::NUMBER_OF_PARAMETERS = 2;

const std::string mitk::LinearModel::UNIT_PARAMETER_b = "[unit of y]/[unit of x]";
const std::string mitk::LinearModel::UNIT_PARAMETER_y0 = "[unit of y]";

const unsigned int mitk::LinearModel::POSITION_PARAMETER_b = 0;
const unsigned int mitk::LinearModel::POSITION_PARAMETER_y0 = 1;

const std::string mitk::LinearModel::NAME_DERIVED_PARAMETER_x_intercept = "x-intercept";

const unsigned int mitk::LinearModel::NUMBER_OF_DERIVED_PARAMETERS = 1;

const std::string mitk::LinearModel::UNIT_DERIVED_PARAMETER_x_intercept = "[unit of x]";

const unsigned int mitk::LinearModel::NUMBER_OF_STATIC_PARAMETERS = 0;

const std::string mitk::LinearModel::MODEL_DISPLAY_NAME = "Linear Model";

const std::string mitk::LinearModel::MODEL_TYPE = "Generic";

const std::string mitk::LinearModel::FUNCTION_STRING = "y(x) = y-intercept + slope*x";

const std::string mitk::LinearModel::X_NAME = "x";

const std::string mitk::LinearModel::X_AXIS_NAME = "X";

const std::string mitk::LinearModel::X_AXIS_UNIT = "unit of x";

const std::string mitk::LinearModel::Y_AXIS_NAME = "Y";

const std::string mitk::LinearModel::Y_AXIS_UNIT = "unit of y";


std::string mitk::LinearModel::GetModelDisplayName() const
{
  return MODEL_DISPLAY_NAME;
};

std::string mitk::LinearModel::GetModelType() const
{
  return MODEL_TYPE;
};

mitk::LinearModel::FunctionStringType mitk::LinearModel::GetFunctionString() const
{
  return FUNCTION_STRING;
};

std::string mitk::LinearModel::GetXName() const
{
  return X_NAME;
};

std::string mitk::LinearModel::GetXAxisName() const
{
  return X_AXIS_NAME;
};

std::string mitk::LinearModel::GetXAxisUnit() const
{
  return X_AXIS_UNIT;
}

std::string mitk::LinearModel::GetYAxisName() const
{
  return Y_AXIS_NAME;
};

std::string mitk::LinearModel::GetYAxisUnit() const
{
  return Y_AXIS_UNIT;
}

mitk::LinearModel::ParameterNamesType
mitk::LinearModel::GetParameterNames() const
{
  ParameterNamesType result;
  result.push_back(NAME_PARAMETER_b);
  result.push_back(NAME_PARAMETER_y0);
  return result;
};

mitk::LinearModel::ParamterUnitMapType mitk::LinearModel::GetParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair(NAME_PARAMETER_b, UNIT_PARAMETER_b));
  result.insert(std::make_pair(NAME_PARAMETER_y0, UNIT_PARAMETER_y0));

  return result;
}

mitk::LinearModel::ParametersSizeType
mitk::LinearModel::GetNumberOfParameters() const
{
  return NUMBER_OF_PARAMETERS;
};

mitk::LinearModel::ParameterNamesType
mitk::LinearModel::GetDerivedParameterNames() const
{
  ParameterNamesType result;
  result.push_back(NAME_DERIVED_PARAMETER_x_intercept);
  return result;
};

mitk::LinearModel::ParametersSizeType
mitk::LinearModel::GetNumberOfDerivedParameters() const
{
  return NUMBER_OF_DERIVED_PARAMETERS;
};

mitk::LinearModel::ParamterUnitMapType mitk::LinearModel::GetDerivedParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair(NAME_DERIVED_PARAMETER_x_intercept, UNIT_DERIVED_PARAMETER_x_intercept));

  return result;
};

mitk::LinearModel::ModelResultType
mitk::LinearModel::ComputeModelfunction(const ParametersType& parameters) const
{
  //Model Parameters
  double     b = parameters[POSITION_PARAMETER_b];
  double     y0 = parameters[POSITION_PARAMETER_y0];

  ModelResultType signal(m_TimeGrid.GetSize());

  TimeGridType::const_iterator timeGridEnd = m_TimeGrid.end();
  ModelResultType::iterator signalPos = signal.begin();

  for (TimeGridType::const_iterator gridPos = m_TimeGrid.begin(); gridPos != timeGridEnd;
       ++gridPos, ++signalPos)
  {
    *signalPos = b * (*gridPos) + y0;
  }

  return signal;
};

mitk::LinearModel::ParameterNamesType mitk::LinearModel::GetStaticParameterNames() const
{
  ParameterNamesType result;

  return result;
}

mitk::LinearModel::ParametersSizeType  mitk::LinearModel::GetNumberOfStaticParameters() const
{
  return NUMBER_OF_STATIC_PARAMETERS;
}

void mitk::LinearModel::SetStaticParameter(const ParameterNameType& /*name*/,
    const StaticParameterValuesType& /*values*/)
{
  //do nothing
};

mitk::LinearModel::StaticParameterValuesType mitk::LinearModel::GetStaticParameterValue(
  const ParameterNameType& /*name*/) const
{
  StaticParameterValuesType result;

  //do nothing

  return result;
};

mitk::ModelBase::DerivedParameterMapType mitk::LinearModel::ComputeDerivedParameters(
  const mitk::ModelBase::ParametersType& parameters) const
{
  DerivedParameterMapType result;

  //Model Parameters
  double     b = parameters[POSITION_PARAMETER_b];
  double     y0 = parameters[POSITION_PARAMETER_y0];

  double intercept = -1 * y0 / b;
  result.insert(std::make_pair(NAME_DERIVED_PARAMETER_x_intercept, intercept));
  return result;
};

itk::LightObject::Pointer mitk::LinearModel::InternalClone() const
{
  LinearModel::Pointer newClone = LinearModel::New();

  newClone->SetTimeGrid(this->m_TimeGrid);

  return newClone.GetPointer();
};
