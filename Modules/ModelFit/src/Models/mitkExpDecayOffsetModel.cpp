/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkExpDecayOffsetModel.h"


const std::string mitk::ExpDecayOffsetModel::NAME_PARAMETER_y0 = "y-intercept";
const std::string mitk::ExpDecayOffsetModel::NAME_PARAMETER_k = "rate";
const std::string mitk::ExpDecayOffsetModel::NAME_PARAMETER_y_bl = "baseline";

const unsigned int mitk::ExpDecayOffsetModel::NUMBER_OF_PARAMETERS = 3;

const std::string mitk::ExpDecayOffsetModel::UNIT_PARAMETER_y0 = "[unit of y]";
const std::string mitk::ExpDecayOffsetModel::UNIT_PARAMETER_k = "1/[unit of x]";
const std::string mitk::ExpDecayOffsetModel::UNIT_PARAMETER_y_bl = "[unit of y]";

const unsigned int mitk::ExpDecayOffsetModel::POSITION_PARAMETER_y0 = 0;
const unsigned int mitk::ExpDecayOffsetModel::POSITION_PARAMETER_k = 1;
const unsigned int mitk::ExpDecayOffsetModel::POSITION_PARAMETER_y_bl = 2;

const unsigned int mitk::ExpDecayOffsetModel::NUMBER_OF_STATIC_PARAMETERS = 0;

const std::string mitk::ExpDecayOffsetModel::MODEL_DISPLAY_NAME = "Exponential Decay Offset Model";

const std::string mitk::ExpDecayOffsetModel::MODEL_TYPE = "Generic";

const std::string mitk::ExpDecayOffsetModel::FUNCTION_STRING = "y(x) = y-intercept * exp(-rate*x) + baseline";

const std::string mitk::ExpDecayOffsetModel::X_NAME = "x";

const std::string mitk::ExpDecayOffsetModel::X_AXIS_NAME = "X";

const std::string mitk::ExpDecayOffsetModel::X_AXIS_UNIT = "unit of x";

const std::string mitk::ExpDecayOffsetModel::Y_AXIS_NAME = "Y";

const std::string mitk::ExpDecayOffsetModel::Y_AXIS_UNIT = "unit of y";

///////////

std::string mitk::ExpDecayOffsetModel::GetModelDisplayName() const
{
  return MODEL_DISPLAY_NAME;
};

std::string mitk::ExpDecayOffsetModel::GetModelType() const
{
  return MODEL_TYPE;
};

mitk::ExpDecayOffsetModel::FunctionStringType mitk::ExpDecayOffsetModel::GetFunctionString() const
{
  return FUNCTION_STRING;
};

std::string mitk::ExpDecayOffsetModel::GetXName() const
{
  return X_NAME;
};

std::string mitk::ExpDecayOffsetModel::GetXAxisName() const
{
  return X_AXIS_NAME;
};

std::string mitk::ExpDecayOffsetModel::GetXAxisUnit() const
{
  return X_AXIS_UNIT;
}

std::string mitk::ExpDecayOffsetModel::GetYAxisName() const
{
  return Y_AXIS_NAME;
};

std::string mitk::ExpDecayOffsetModel::GetYAxisUnit() const
{
  return Y_AXIS_UNIT;
}

mitk::ExpDecayOffsetModel::ParameterNamesType
mitk::ExpDecayOffsetModel::GetParameterNames() const
{
  ParameterNamesType result;
  result.push_back(NAME_PARAMETER_y0);
  result.push_back(NAME_PARAMETER_k);
  result.push_back(NAME_PARAMETER_y_bl);
  return result;
};

mitk::ExpDecayOffsetModel::ParamterUnitMapType mitk::ExpDecayOffsetModel::GetParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair(NAME_PARAMETER_y0, UNIT_PARAMETER_y0));
  result.insert(std::make_pair(NAME_PARAMETER_k, UNIT_PARAMETER_k));
  result.insert(std::make_pair(NAME_PARAMETER_y_bl, UNIT_PARAMETER_y_bl));

  return result;
}

mitk::ExpDecayOffsetModel::ParametersSizeType
mitk::ExpDecayOffsetModel::GetNumberOfParameters() const
{
  return NUMBER_OF_PARAMETERS;
};

mitk::ExpDecayOffsetModel::ModelResultType
mitk::ExpDecayOffsetModel::ComputeModelfunction(const ParametersType& parameters) const
{
  ModelResultType signal(m_TimeGrid.GetSize());

  const auto timeGridEnd = m_TimeGrid.end();
  ModelResultType::iterator signalPos = signal.begin();

  for (auto gridPos = m_TimeGrid.begin(); gridPos != timeGridEnd;
       ++gridPos, ++signalPos)
  {
    *signalPos = parameters[0] * exp(-1.0 * (*gridPos) * parameters[1]) + parameters[2];
  }

  return signal;
};

mitk::ExpDecayOffsetModel::ParameterNamesType mitk::ExpDecayOffsetModel::GetStaticParameterNames() const
{
  return {};
}

mitk::ExpDecayOffsetModel::ParametersSizeType  mitk::ExpDecayOffsetModel::GetNumberOfStaticParameters() const
{
  return NUMBER_OF_STATIC_PARAMETERS;
}

void mitk::ExpDecayOffsetModel::SetStaticParameter(const ParameterNameType& /*name*/,
    const StaticParameterValuesType& /*values*/)
{
  //do nothing
};

mitk::ExpDecayOffsetModel::StaticParameterValuesType mitk::ExpDecayOffsetModel::GetStaticParameterValue(
  const ParameterNameType& /*name*/) const
{
  StaticParameterValuesType result;

  //do nothing

  return result;
};

itk::LightObject::Pointer mitk::ExpDecayOffsetModel::InternalClone() const
{
  ExpDecayOffsetModel::Pointer newClone = ExpDecayOffsetModel::New();

  newClone->SetTimeGrid(this->m_TimeGrid);

  return newClone.GetPointer();
};
