/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkExponentialSaturationModel.h"
#include "mitkNumericConstants.h"

const std::string mitk::ExponentialSaturationModel::NAME_PARAMETER_BAT = "onset";
const std::string mitk::ExponentialSaturationModel::NAME_PARAMETER_y_bl = "baseline";
const std::string mitk::ExponentialSaturationModel::NAME_PARAMETER_y_fin = "y_final";
const std::string mitk::ExponentialSaturationModel::NAME_PARAMETER_k = "rate";

const unsigned int mitk::ExponentialSaturationModel::NUMBER_OF_PARAMETERS = 4;

const std::string mitk::ExponentialSaturationModel::UNIT_PARAMETER_BAT = "[unit of x]";
const std::string mitk::ExponentialSaturationModel::UNIT_PARAMETER_y_bl = "[unit of y]";
const std::string mitk::ExponentialSaturationModel::UNIT_PARAMETER_y_fin = "[unit of y]";
const std::string mitk::ExponentialSaturationModel::UNIT_PARAMETER_k = "1/[unit of x]";

const unsigned int mitk::ExponentialSaturationModel::POSITION_PARAMETER_BAT = 0;
const unsigned int mitk::ExponentialSaturationModel::POSITION_PARAMETER_y_bl = 1;
const unsigned int mitk::ExponentialSaturationModel::POSITION_PARAMETER_y_fin = 2;
const unsigned int mitk::ExponentialSaturationModel::POSITION_PARAMETER_k = 3;

const unsigned int mitk::ExponentialSaturationModel::NUMBER_OF_STATIC_PARAMETERS = 0;

const std::string mitk::ExponentialSaturationModel::MODEL_DISPLAY_NAME = "Exponential Saturation Model";

const std::string mitk::ExponentialSaturationModel::MODEL_TYPE = "Generic";

const std::string mitk::ExponentialSaturationModel::FUNCTION_STRING = "if x<onset: y(x) = baseline , else: y(x) = baseline + (y_final-baseline) * (1 - exp(-rate*(x-onset)))";

const std::string mitk::ExponentialSaturationModel::X_NAME = "x";

const std::string mitk::ExponentialSaturationModel::X_AXIS_NAME = "X";

const std::string mitk::ExponentialSaturationModel::X_AXIS_UNIT = "unit of x";

const std::string mitk::ExponentialSaturationModel::Y_AXIS_NAME = "Y";

const std::string mitk::ExponentialSaturationModel::Y_AXIS_UNIT = "unit of y";

std::string mitk::ExponentialSaturationModel::GetModelDisplayName() const
{
  return MODEL_DISPLAY_NAME;
};

std::string mitk::ExponentialSaturationModel::GetModelType() const
{
  return MODEL_TYPE;
};

mitk::ExponentialSaturationModel::FunctionStringType mitk::ExponentialSaturationModel::GetFunctionString() const
{
  return FUNCTION_STRING;
};

std::string mitk::ExponentialSaturationModel::GetXName() const
{
  return X_NAME;
};

std::string mitk::ExponentialSaturationModel::GetXAxisName() const
{
  return X_AXIS_NAME;
};

std::string mitk::ExponentialSaturationModel::GetXAxisUnit() const
{
  return X_AXIS_UNIT;
}

std::string mitk::ExponentialSaturationModel::GetYAxisName() const
{
  return Y_AXIS_NAME;
};

std::string mitk::ExponentialSaturationModel::GetYAxisUnit() const
{
  return Y_AXIS_UNIT;
}

mitk::ExponentialSaturationModel::ParameterNamesType
mitk::ExponentialSaturationModel::GetParameterNames() const
{
  ParameterNamesType result;
  result.push_back(NAME_PARAMETER_BAT);
  result.push_back(NAME_PARAMETER_y_bl);
  result.push_back(NAME_PARAMETER_y_fin);
  result.push_back(NAME_PARAMETER_k);
  return result;
};

mitk::ExponentialSaturationModel::ParamterUnitMapType mitk::ExponentialSaturationModel::GetParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair(NAME_PARAMETER_BAT, UNIT_PARAMETER_BAT));
  result.insert(std::make_pair(NAME_PARAMETER_y_bl, UNIT_PARAMETER_y_bl));
  result.insert(std::make_pair(NAME_PARAMETER_y_fin, UNIT_PARAMETER_y_fin));
  result.insert(std::make_pair(NAME_PARAMETER_k, UNIT_PARAMETER_k));

  return result;
}

mitk::ExponentialSaturationModel::ParametersSizeType
mitk::ExponentialSaturationModel::GetNumberOfParameters() const
{
  return NUMBER_OF_PARAMETERS;
};

mitk::ExponentialSaturationModel::ModelResultType
mitk::ExponentialSaturationModel::ComputeModelfunction(const ParametersType& parameters) const
{
  ModelResultType signal(m_TimeGrid.GetSize());

  ModelResultType::iterator signalPos = signal.begin();

  for (const auto& gridPos : m_TimeGrid)
  {
    if ((gridPos) < parameters[0])
    {
      *signalPos = parameters[1];
    }
    else
    {
      *signalPos = parameters[1] + (parameters[2]- parameters[1]) * (1.0 - exp((-1.0) * parameters[3] * (gridPos - parameters[0])));
    }

    ++signalPos;
  }

  return signal;
};

mitk::ExponentialSaturationModel::ParameterNamesType mitk::ExponentialSaturationModel::GetStaticParameterNames() const
{
  ParameterNamesType result;

  return result;
}

mitk::ExponentialSaturationModel::ParametersSizeType  mitk::ExponentialSaturationModel::GetNumberOfStaticParameters() const
{
  return NUMBER_OF_STATIC_PARAMETERS;
}

void mitk::ExponentialSaturationModel::SetStaticParameter(const ParameterNameType& /*name*/,
    const StaticParameterValuesType& /*values*/)
{
  //do nothing
};

mitk::ExponentialSaturationModel::StaticParameterValuesType mitk::ExponentialSaturationModel::GetStaticParameterValue(
  const ParameterNameType& /*name*/) const
{
  StaticParameterValuesType result;

  //do nothing

  return result;
};

itk::LightObject::Pointer mitk::ExponentialSaturationModel::InternalClone() const
{
  ExponentialSaturationModel::Pointer newClone = ExponentialSaturationModel::New();

  newClone->SetTimeGrid(this->m_TimeGrid);

  return newClone.GetPointer();
};
