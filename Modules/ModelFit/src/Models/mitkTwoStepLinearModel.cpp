/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTwoStepLinearModel.h"
#include <mitkIOUtil.h>


const std::string mitk::TwoStepLinearModel::NAME_PARAMETER_y0 = "y-intercept";
const std::string mitk::TwoStepLinearModel::NAME_PARAMETER_x0 = "x_changepoint";
const std::string mitk::TwoStepLinearModel::NAME_PARAMETER_b0 = "slope1";
const std::string mitk::TwoStepLinearModel::NAME_PARAMETER_b1 = "slope2";

const unsigned int mitk::TwoStepLinearModel::NUMBER_OF_PARAMETERS = 4;

const std::string mitk::TwoStepLinearModel::UNIT_PARAMETER_y0 = "[unit of y]";
const std::string mitk::TwoStepLinearModel::UNIT_PARAMETER_x0 = "[unit of x]";
const std::string mitk::TwoStepLinearModel::UNIT_PARAMETER_b0 = "[unit of y]/[unit of x]";
const std::string mitk::TwoStepLinearModel::UNIT_PARAMETER_b1 = "[unit of y]/[unit of x]";

const unsigned int mitk::TwoStepLinearModel::POSITION_PARAMETER_y0 = 0;
const unsigned int mitk::TwoStepLinearModel::POSITION_PARAMETER_x0 = 1;
const unsigned int mitk::TwoStepLinearModel::POSITION_PARAMETER_b0 = 2;
const unsigned int mitk::TwoStepLinearModel::POSITION_PARAMETER_b1 = 3;

const std::string mitk::TwoStepLinearModel::NAME_DERIVED_PARAMETER_auc = "auc";
const std::string mitk::TwoStepLinearModel::NAME_DERIVED_PARAMETER_y_fin = "y_final";
const std::string mitk::TwoStepLinearModel::NAME_DERIVED_PARAMETER_y_max = "y_max";
const std::string mitk::TwoStepLinearModel::NAME_DERIVED_PARAMETER_y1 = "y-intercept1";

const unsigned int mitk::TwoStepLinearModel::NUMBER_OF_DERIVED_PARAMETERS = 4;

const std::string mitk::TwoStepLinearModel::UNIT_DERIVED_PARAMETER_auc = "[unit of x]*[unit of y]";
const std::string mitk::TwoStepLinearModel::UNIT_DERIVED_PARAMETER_y_fin = "[unit of y]";
const std::string mitk::TwoStepLinearModel::UNIT_DERIVED_PARAMETER_y_max = "[unit of y]";
const std::string mitk::TwoStepLinearModel::UNIT_DERIVED_PARAMETER_y1 = "[unit of y]";

const unsigned int mitk::TwoStepLinearModel::NUMBER_OF_STATIC_PARAMETERS = 0;

const std::string mitk::TwoStepLinearModel::MODEL_DISPLAY_NAME = "Two Step Linear Model";

const std::string mitk::TwoStepLinearModel::MODEL_TYPE = "Generic";

const std::string mitk::TwoStepLinearModel::FUNCTION_STRING = "if x < x_changepoint: y(x) = y-intercept + slope1*x, else: y(x) = y-intercept1 + slope2*x";

const std::string mitk::TwoStepLinearModel::X_NAME = "x";

const std::string mitk::TwoStepLinearModel::X_AXIS_NAME = "X";

const std::string mitk::TwoStepLinearModel::X_AXIS_UNIT = "unit of x";

const std::string mitk::TwoStepLinearModel::Y_AXIS_NAME = "Y";

const std::string mitk::TwoStepLinearModel::Y_AXIS_UNIT = "unit of y";


std::string mitk::TwoStepLinearModel::GetModelDisplayName() const
{
  return MODEL_DISPLAY_NAME;
};

std::string mitk::TwoStepLinearModel::GetModelType() const
{
  return MODEL_TYPE;
};

mitk::TwoStepLinearModel::FunctionStringType mitk::TwoStepLinearModel::GetFunctionString() const
{
  return FUNCTION_STRING;
};

std::string mitk::TwoStepLinearModel::GetXName() const
{
  return X_NAME;
};

std::string mitk::TwoStepLinearModel::GetXAxisName() const
{
  return X_AXIS_NAME;
};

std::string mitk::TwoStepLinearModel::GetXAxisUnit() const
{
  return X_AXIS_UNIT;
}

std::string mitk::TwoStepLinearModel::GetYAxisName() const
{
  return Y_AXIS_NAME;
};

std::string mitk::TwoStepLinearModel::GetYAxisUnit() const
{
  return Y_AXIS_UNIT;
}

mitk::TwoStepLinearModel::ParameterNamesType
mitk::TwoStepLinearModel::GetParameterNames() const
{
  ParameterNamesType result;
  result.push_back(NAME_PARAMETER_y0);
  result.push_back(NAME_PARAMETER_x0);
  result.push_back(NAME_PARAMETER_b0);
  result.push_back(NAME_PARAMETER_b1);
  return result;
};

mitk::TwoStepLinearModel::ParamterUnitMapType mitk::TwoStepLinearModel::GetParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair(NAME_PARAMETER_y0, UNIT_PARAMETER_y0));
  result.insert(std::make_pair(NAME_PARAMETER_x0, UNIT_PARAMETER_x0));
  result.insert(std::make_pair(NAME_PARAMETER_b0, UNIT_PARAMETER_b0));
  result.insert(std::make_pair(NAME_PARAMETER_b1, UNIT_PARAMETER_b1));

  return result;
}

mitk::TwoStepLinearModel::ParametersSizeType
mitk::TwoStepLinearModel::GetNumberOfParameters() const
{
  return NUMBER_OF_PARAMETERS;
};

mitk::TwoStepLinearModel::ParameterNamesType
mitk::TwoStepLinearModel::GetDerivedParameterNames() const
{
  ParameterNamesType result;
  result.push_back(NAME_DERIVED_PARAMETER_auc);
  result.push_back(NAME_DERIVED_PARAMETER_y_fin);
  result.push_back(NAME_DERIVED_PARAMETER_y_max);
  result.push_back(NAME_DERIVED_PARAMETER_y1);
  return result;
};

mitk::TwoStepLinearModel::ParametersSizeType
mitk::TwoStepLinearModel::GetNumberOfDerivedParameters() const
{
  return NUMBER_OF_DERIVED_PARAMETERS;
};

mitk::TwoStepLinearModel::ParamterUnitMapType mitk::TwoStepLinearModel::GetDerivedParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair(NAME_DERIVED_PARAMETER_auc, UNIT_DERIVED_PARAMETER_auc));
  result.insert(std::make_pair(NAME_DERIVED_PARAMETER_y_fin, UNIT_DERIVED_PARAMETER_y_fin));
  result.insert(std::make_pair(NAME_DERIVED_PARAMETER_y_max, UNIT_DERIVED_PARAMETER_y_max));
  result.insert(std::make_pair(NAME_DERIVED_PARAMETER_y1, UNIT_DERIVED_PARAMETER_y1));

  return result;
};

double mitk::TwoStepLinearModel::ComputeSignalFromParameters(double x, double x0, double b0, double b1, double y0, double y1)
{
  return (x < x0) ? (b0 * x + y0) : (b1 * x + y1);
};




mitk::TwoStepLinearModel::ModelResultType
mitk::TwoStepLinearModel::ComputeModelfunction(const ParametersType& parameters) const
{

  //Model Parameters
  const auto y0 = parameters[POSITION_PARAMETER_y0];
  const auto x0 = parameters[POSITION_PARAMETER_x0] ;
  const auto b0 = parameters[POSITION_PARAMETER_b0] ;
  const auto b1 = parameters[POSITION_PARAMETER_b1] ;

  double y1 = (b0 - b1) * x0 + y0;

  ModelResultType signal(m_TimeGrid.GetSize());

  TimeGridType::const_iterator timeGridEnd = m_TimeGrid.end();

  ModelResultType::iterator signalPos = signal.begin();

  for (TimeGridType::const_iterator gridPos = m_TimeGrid.begin(); gridPos != timeGridEnd; ++gridPos, ++signalPos)
  {
    *signalPos = ComputeSignalFromParameters(*gridPos, x0, b0, b1, y0, y1);
  }

  return signal;
};

mitk::TwoStepLinearModel::ParameterNamesType mitk::TwoStepLinearModel::GetStaticParameterNames() const
{
  ParameterNamesType result;

  return result;
}

mitk::TwoStepLinearModel::ParametersSizeType  mitk::TwoStepLinearModel::GetNumberOfStaticParameters() const
{
  return 0;
}

void mitk::TwoStepLinearModel::SetStaticParameter(const ParameterNameType& /*name*/,
    const StaticParameterValuesType& /*values*/)
{
  //do nothing
};

mitk::TwoStepLinearModel::StaticParameterValuesType mitk::TwoStepLinearModel::GetStaticParameterValue(
  const ParameterNameType& /*name*/) const
{
  StaticParameterValuesType result;

  //do nothing

  return result;
};

mitk::ModelBase::DerivedParameterMapType mitk::TwoStepLinearModel::ComputeDerivedParameters(
  const mitk::ModelBase::ParametersType& parameters) const
{
    const auto y0 = parameters[POSITION_PARAMETER_y0];
    const auto x0 = parameters[POSITION_PARAMETER_x0] ;
    const auto b0 = parameters[POSITION_PARAMETER_b0] ;
    const auto b1 = parameters[POSITION_PARAMETER_b1] ;
    const auto y1 = (b0 - b1) * x0 + y0;

    unsigned int timeSteps = m_TimeGrid.GetSize();

    const double taq = (m_TimeGrid.empty() == false) ? (m_TimeGrid.GetElement(timeSteps - 1)) : (mitkThrow() << "An exception occured because time grid is empty, method can't continue.");

    const double y_fin = b1 * taq + y1;

    double y_max = y_fin;
    if ((b0 >= 0) && (b1 >= 0))
      y_max = y_fin;
    else if ((b0 < 0) && (b1 < 0))
      y_max = y0;
    else if ((b0 > 0) && (b1 < 0))
      y_max = (b0 * x0 + y0);
    else
    {
      if (abs(b0 * x0) >= abs(b1 * (taq - x0)))
        y_max = y0;
      else y_max = y_fin;
    }

    double auc = 0.0;
    TimeGridType::const_iterator timeGridEnd = m_TimeGrid.end();
    for (TimeGridType::const_iterator gridPos = m_TimeGrid.begin(); gridPos != timeGridEnd -1; ++gridPos)
    {
      double currentGridPos = *gridPos;
      double nextGridPos = *(++gridPos);
      double deltaX = nextGridPos - currentGridPos;
      double deltaY = ComputeSignalFromParameters(nextGridPos, x0, b0, b1, y0, y1) - ComputeSignalFromParameters(currentGridPos, x0, b0, b1, y0, y1);
      double Yi = ComputeSignalFromParameters(currentGridPos, x0, b0, b1, y0, y1);
      double intI = 0.5 * deltaX * deltaY + Yi * deltaX;
      auc += std::abs(intI);
      --gridPos;
    }

    DerivedParameterMapType result;

    result.insert(std::make_pair(NAME_DERIVED_PARAMETER_auc, auc));
    result.insert(std::make_pair(NAME_DERIVED_PARAMETER_y_fin, y_fin));
    result.insert(std::make_pair(NAME_DERIVED_PARAMETER_y_max, y_max));
    result.insert(std::make_pair(NAME_DERIVED_PARAMETER_y1, y1));

    return result;
};

itk::LightObject::Pointer mitk::TwoStepLinearModel::InternalClone() const
{
  TwoStepLinearModel::Pointer newClone = TwoStepLinearModel::New();

  newClone->SetTimeGrid(this->m_TimeGrid);

  return newClone.GetPointer();
};
