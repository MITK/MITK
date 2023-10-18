/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkThreeStepLinearModel.h"
#include <mitkIOUtil.h>

const std::string mitk::ThreeStepLinearModel::NAME_PARAMETER_y_bl = "baseline";
const std::string mitk::ThreeStepLinearModel::NAME_PARAMETER_x0 = "x_changepoint1";
const std::string mitk::ThreeStepLinearModel::NAME_PARAMETER_x1 = "x_changepoint2";
const std::string mitk::ThreeStepLinearModel::NAME_PARAMETER_b0 = "slope1";
const std::string mitk::ThreeStepLinearModel::NAME_PARAMETER_b1 = "slope2";

const unsigned int mitk::ThreeStepLinearModel::NUMBER_OF_PARAMETERS = 5;

const std::string mitk::ThreeStepLinearModel::UNIT_PARAMETER_y_bl = "[unit of y]";
const std::string mitk::ThreeStepLinearModel::UNIT_PARAMETER_x0 = "[unit of x]";
const std::string mitk::ThreeStepLinearModel::UNIT_PARAMETER_x1 = "[unit of x]";
const std::string mitk::ThreeStepLinearModel::UNIT_PARAMETER_b0 = "[unit of y]/[unit of x]";
const std::string mitk::ThreeStepLinearModel::UNIT_PARAMETER_b1 = "[unit of y]/[unit of x]";

const unsigned int mitk::ThreeStepLinearModel::POSITION_PARAMETER_y_bl = 0;
const unsigned int mitk::ThreeStepLinearModel::POSITION_PARAMETER_x0 = 1;
const unsigned int mitk::ThreeStepLinearModel::POSITION_PARAMETER_x1 = 2;
const unsigned int mitk::ThreeStepLinearModel::POSITION_PARAMETER_b0 = 3;
const unsigned int mitk::ThreeStepLinearModel::POSITION_PARAMETER_b1 = 4;

const std::string mitk::ThreeStepLinearModel::NAME_DERIVED_PARAMETER_auc = "auc";
const std::string mitk::ThreeStepLinearModel::NAME_DERIVED_PARAMETER_x_fin = "x_final";
const std::string mitk::ThreeStepLinearModel::NAME_DERIVED_PARAMETER_y_fin = "y_final";
const std::string mitk::ThreeStepLinearModel::NAME_DERIVED_PARAMETER_y_max = "y_max";
const std::string mitk::ThreeStepLinearModel::NAME_DERIVED_PARAMETER_y1 = "y-intercept1";
const std::string mitk::ThreeStepLinearModel::NAME_DERIVED_PARAMETER_y2 = "y-intercept2";

const unsigned int mitk::ThreeStepLinearModel::NUMBER_OF_DERIVED_PARAMETERS = 6;

const std::string mitk::ThreeStepLinearModel::UNIT_DERIVED_PARAMETER_auc = "[unit of x]*[unit of y]";
const std::string mitk::ThreeStepLinearModel::UNIT_DERIVED_PARAMETER_x_fin = "[unit of x]";
const std::string mitk::ThreeStepLinearModel::UNIT_DERIVED_PARAMETER_y_fin = "[unit of y]";
const std::string mitk::ThreeStepLinearModel::UNIT_DERIVED_PARAMETER_y_max = "[unit of y]";
const std::string mitk::ThreeStepLinearModel::UNIT_DERIVED_PARAMETER_y1 = "[unit of y]";
const std::string mitk::ThreeStepLinearModel::UNIT_DERIVED_PARAMETER_y2 = "[unit of y]";

const unsigned int mitk::ThreeStepLinearModel::NUMBER_OF_STATIC_PARAMETERS = 0;

const std::string mitk::ThreeStepLinearModel::MODEL_DISPLAY_NAME = "Three Step Linear Model";

const std::string mitk::ThreeStepLinearModel::MODEL_TYPE = "Generic";

const std::string mitk::ThreeStepLinearModel::FUNCTION_STRING = "if x < x_changepoint1: y(x) = baseline, else if x_changepoint1 <= x <= x_changepoint2: y(x) = y-intercept1 + slope1*x, else if x>x_changepoint2: y(x) = y-intercept2 + slope2*x";

const std::string mitk::ThreeStepLinearModel::X_NAME = "x";

const std::string mitk::ThreeStepLinearModel::X_AXIS_NAME = "X";

const std::string mitk::ThreeStepLinearModel::X_AXIS_UNIT = "unit of x";

const std::string mitk::ThreeStepLinearModel::Y_AXIS_NAME = "Y";

const std::string mitk::ThreeStepLinearModel::Y_AXIS_UNIT = "unit of y";


std::string mitk::ThreeStepLinearModel::GetModelDisplayName() const
{
  return MODEL_DISPLAY_NAME;
};

std::string mitk::ThreeStepLinearModel::GetModelType() const
{
  return MODEL_TYPE;
};

mitk::ThreeStepLinearModel::FunctionStringType mitk::ThreeStepLinearModel::GetFunctionString() const
{
  return FUNCTION_STRING;
};

std::string mitk::ThreeStepLinearModel::GetXName() const
{
  return X_NAME;
};

std::string mitk::ThreeStepLinearModel::GetXAxisName() const
{
  return X_AXIS_NAME;
};

std::string mitk::ThreeStepLinearModel::GetXAxisUnit() const
{
  return X_AXIS_UNIT;
}

std::string mitk::ThreeStepLinearModel::GetYAxisName() const
{
  return Y_AXIS_NAME;
};

std::string mitk::ThreeStepLinearModel::GetYAxisUnit() const
{
  return Y_AXIS_UNIT;
}

mitk::ThreeStepLinearModel::ParameterNamesType
mitk::ThreeStepLinearModel::GetParameterNames() const
{
  ParameterNamesType result;
  result.push_back(NAME_PARAMETER_y_bl);
  result.push_back(NAME_PARAMETER_x0);
  result.push_back(NAME_PARAMETER_x1);
  result.push_back(NAME_PARAMETER_b0);
  result.push_back(NAME_PARAMETER_b1);
  return result;
};

mitk::ThreeStepLinearModel::ParamterUnitMapType mitk::ThreeStepLinearModel::GetParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair(NAME_PARAMETER_y_bl, UNIT_PARAMETER_y_bl));
  result.insert(std::make_pair(NAME_PARAMETER_x0, UNIT_PARAMETER_x0));
  result.insert(std::make_pair(NAME_PARAMETER_x1, UNIT_PARAMETER_x1));
  result.insert(std::make_pair(NAME_PARAMETER_b0, UNIT_PARAMETER_b0));
  result.insert(std::make_pair(NAME_PARAMETER_b1, UNIT_PARAMETER_b1));

  return result;
}

mitk::ThreeStepLinearModel::ParametersSizeType
mitk::ThreeStepLinearModel::GetNumberOfParameters() const
{
  return NUMBER_OF_PARAMETERS;
};

mitk::ThreeStepLinearModel::ParameterNamesType
mitk::ThreeStepLinearModel::GetDerivedParameterNames() const
{
  ParameterNamesType result;
  result.push_back(NAME_DERIVED_PARAMETER_auc);
  result.push_back(NAME_DERIVED_PARAMETER_x_fin);
  result.push_back(NAME_DERIVED_PARAMETER_y_fin);
  result.push_back(NAME_DERIVED_PARAMETER_y_max);
  result.push_back(NAME_DERIVED_PARAMETER_y1);
  result.push_back(NAME_DERIVED_PARAMETER_y2);
  return result;
};

mitk::ThreeStepLinearModel::ParametersSizeType
mitk::ThreeStepLinearModel::GetNumberOfDerivedParameters() const
{
  return NUMBER_OF_DERIVED_PARAMETERS;
};

mitk::ThreeStepLinearModel::ParamterUnitMapType mitk::ThreeStepLinearModel::GetDerivedParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair(NAME_DERIVED_PARAMETER_auc, UNIT_DERIVED_PARAMETER_auc));
  result.insert(std::make_pair(NAME_DERIVED_PARAMETER_x_fin, UNIT_DERIVED_PARAMETER_x_fin));
  result.insert(std::make_pair(NAME_DERIVED_PARAMETER_y_fin, UNIT_DERIVED_PARAMETER_y_fin));
  result.insert(std::make_pair(NAME_DERIVED_PARAMETER_y_max, UNIT_DERIVED_PARAMETER_y_max));
  result.insert(std::make_pair(NAME_DERIVED_PARAMETER_y1, UNIT_DERIVED_PARAMETER_y1));
  result.insert(std::make_pair(NAME_DERIVED_PARAMETER_y2, UNIT_DERIVED_PARAMETER_y2));

  return result;
};


double mitk::ThreeStepLinearModel::ComputeSignalFromParameters(double x, double y_bl, double x0, double x1, double b0, double b1, double y1, double y2)
{
  double signal = 0.0;

  if (x < x0)
  {
    signal = y_bl;
  }
  else if (x >= x0 && x <= x1)
  {
    signal = b0 * x + y1;
  }
  else
  {
    signal = b1 * x + y2;
  }


  return signal;
};


mitk::ThreeStepLinearModel::ModelResultType
mitk::ThreeStepLinearModel::ComputeModelfunction(const ParametersType& parameters) const
{
  //Model Parameters
  const double     y_bl = (double) parameters[POSITION_PARAMETER_y_bl];
  const double     x0 = (double) parameters[POSITION_PARAMETER_x0] ;
  const double     x1 = (double) parameters[POSITION_PARAMETER_x1] ;
  const double     b0 = (double) parameters[POSITION_PARAMETER_b0] ;
  const double     b1 = (double) parameters[POSITION_PARAMETER_b1] ;

  double     y1 = y_bl - b0 * x0;
  double     y2 = (b0 * x1 + y1) - (b1 * x1);

  ModelResultType signal(m_TimeGrid.GetSize());

  TimeGridType::const_iterator timeGridEnd = m_TimeGrid.end();

  ModelResultType::iterator signalPos = signal.begin();

  for (TimeGridType::const_iterator gridPos = m_TimeGrid.begin(); gridPos != timeGridEnd; ++gridPos, ++signalPos)
  {
    *signalPos = ComputeSignalFromParameters(*gridPos, y_bl, x0, x1, b0, b1, y1, y2);
  }

  return signal;
};

mitk::ThreeStepLinearModel::ParameterNamesType mitk::ThreeStepLinearModel::GetStaticParameterNames() const
{
  ParameterNamesType result;

  return result;
}

mitk::ThreeStepLinearModel::ParametersSizeType  mitk::ThreeStepLinearModel::GetNumberOfStaticParameters() const
{
  return 0;
}

void mitk::ThreeStepLinearModel::SetStaticParameter(const ParameterNameType&,
    const StaticParameterValuesType&)
{
  //do nothing
};

mitk::ThreeStepLinearModel::StaticParameterValuesType mitk::ThreeStepLinearModel::GetStaticParameterValue(
  const ParameterNameType&) const
{
  StaticParameterValuesType result;

  //do nothing

  return result;
};

mitk::ModelBase::DerivedParameterMapType mitk::ThreeStepLinearModel::ComputeDerivedParameters(
  const mitk::ModelBase::ParametersType& parameters) const
{
    const double     y_bl = (double) parameters[POSITION_PARAMETER_y_bl];
    const double     x0 = (double) parameters[POSITION_PARAMETER_x0] ;
    const double     x1 = (double) parameters[POSITION_PARAMETER_x1] ;
    const double     b0 = (double) parameters[POSITION_PARAMETER_b0] ;
    const double     b1 = (double) parameters[POSITION_PARAMETER_b1] ;

    const double     y1 = y_bl - b0 * x0;
    const double     y2 = (b0 * x1 + y1) - (b1 * x1);

    unsigned int timeSteps = m_TimeGrid.GetSize();

    const double x_fin = (m_TimeGrid.empty() == false) ? (m_TimeGrid.GetElement(timeSteps - 1)) : ( mitkThrow() << "An exception occured because time grid is empty, method can't continue.");

    const double y_fin = b1 * x_fin + y2;

    double y_max = y_fin;
    if ((b0 >= 0) && (b1 >= 0))
      y_max = y_fin;
    else if ((b0 < 0) && (b1 < 0))
      y_max = y_bl;
    else if ((b0 > 0) && (b1 < 0))
      y_max = (b0 * x1 + y1);
    else
    {
      if (abs(b0 * (x1 - x0)) >= abs(b1 * (x_fin - x1)))
        y_max = y_bl;
      else y_max = y_fin;
    }

    double auc = 0.0;
    TimeGridType::const_iterator timeGridEnd = m_TimeGrid.end();

    for (TimeGridType::const_iterator gridPos = m_TimeGrid.begin(); gridPos != timeGridEnd - 1; ++gridPos)
    {
      double currentGridPos = *gridPos;
      double nextGridPos = *(++gridPos);
      double deltaX = nextGridPos - currentGridPos;
      double deltaY = ComputeSignalFromParameters(nextGridPos, y_bl, x0, x1, b0, b1, y1, y2) - ComputeSignalFromParameters(currentGridPos, y_bl, x0, x1, b0, b1, y1, y2);
      double Yi = ComputeSignalFromParameters(currentGridPos, y_bl, x0, x1, b0, b1, y1, y2 );
      double intI = 0.5 * deltaX * deltaY + Yi * deltaX;
      auc += std::abs(intI);
      --gridPos;
    }


    DerivedParameterMapType result;

    result.insert(std::make_pair(NAME_DERIVED_PARAMETER_auc, auc));
    result.insert(std::make_pair(NAME_DERIVED_PARAMETER_x_fin, x_fin));
    result.insert(std::make_pair(NAME_DERIVED_PARAMETER_y_fin, y_fin));
    result.insert(std::make_pair(NAME_DERIVED_PARAMETER_y_max, y_max));
    result.insert(std::make_pair(NAME_DERIVED_PARAMETER_y1, y1));
    result.insert(std::make_pair(NAME_DERIVED_PARAMETER_y2, y2));


    return result;
};

itk::LightObject::Pointer mitk::ThreeStepLinearModel::InternalClone() const
{
  ThreeStepLinearModel::Pointer newClone = ThreeStepLinearModel::New();

  newClone->SetTimeGrid(this->m_TimeGrid);

  return newClone.GetPointer();
};
