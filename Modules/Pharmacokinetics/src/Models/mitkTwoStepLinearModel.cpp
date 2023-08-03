/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTwoStepLinearModel.h"
#include <mitkIOUtil.h>

const std::string mitk::TwoStepLinearModel::MODELL_NAME = "Two Step Linear Model";

const std::string mitk::TwoStepLinearModel::NAME_PARAMETER_y1 = "BaseValue";
const std::string mitk::TwoStepLinearModel::NAME_PARAMETER_a1 = "Slope_1";
const std::string mitk::TwoStepLinearModel::NAME_PARAMETER_t = "Change_Point";
const std::string mitk::TwoStepLinearModel::NAME_PARAMETER_a2 = "Slope_2";

const unsigned int mitk::TwoStepLinearModel::POSITION_PARAMETER_y1 = 0;
const unsigned int mitk::TwoStepLinearModel::POSITION_PARAMETER_t  = 1;
const unsigned int mitk::TwoStepLinearModel::POSITION_PARAMETER_a1 = 2;
const unsigned int mitk::TwoStepLinearModel::POSITION_PARAMETER_a2 = 3;

const unsigned int mitk::TwoStepLinearModel::NUMBER_OF_PARAMETERS = 4;

std::string mitk::TwoStepLinearModel::GetModelDisplayName() const
{
  return MODELL_NAME;
};

std::string mitk::TwoStepLinearModel::GetModelType() const
{
  return "Generic";
};

mitk::TwoStepLinearModel::FunctionStringType mitk::TwoStepLinearModel::GetFunctionString() const
{
  return "Slope_1*t+Y_intercept_1 if t<Change_Point; Slope_2*t+Y_intercept_2 if ChangePoint<=t";
};

std::string mitk::TwoStepLinearModel::GetXName() const
{
  return "x";
};

mitk::TwoStepLinearModel::ParameterNamesType
mitk::TwoStepLinearModel::GetParameterNames() const
{
  ParameterNamesType result;
  result.push_back(NAME_PARAMETER_y1);
  result.push_back(NAME_PARAMETER_t);
  result.push_back(NAME_PARAMETER_a1);
  result.push_back(NAME_PARAMETER_a2);


  return result;
};

mitk::TwoStepLinearModel::ParametersSizeType
mitk::TwoStepLinearModel::GetNumberOfParameters() const
{
  return NUMBER_OF_PARAMETERS;
};

mitk::TwoStepLinearModel::ParameterNamesType
mitk::TwoStepLinearModel::GetDerivedParameterNames() const
{
  ParameterNamesType result;
  result.push_back("AUC");
  result.push_back("FinalUptake");
  result.push_back("Smax");
  result.push_back("y-intercept2");

  return result;
};

mitk::TwoStepLinearModel::ParametersSizeType
mitk::TwoStepLinearModel::GetNumberOfDerivedParameters() const
{
  return 4;
};

double mitk::TwoStepLinearModel::ComputeSignalFromParameters(double x, double t, double a1, double a2, double b1, double b2)
{
  return (x < t) ? (a1 * x + b1) : (a2 * x + b2);
};




mitk::TwoStepLinearModel::ModelResultType
mitk::TwoStepLinearModel::ComputeModelfunction(const ParametersType& parameters) const
{

  //Model Parameters
  const auto t = parameters[POSITION_PARAMETER_t] ;
  const auto a1 = parameters[POSITION_PARAMETER_a1] ;
  const auto a2 = parameters[POSITION_PARAMETER_a2] ;
  const auto b1 = parameters[POSITION_PARAMETER_y1] ;

  double b2 = (a1 - a2) * t + b1;

  ModelResultType signal(m_TimeGrid.GetSize());

  TimeGridType::const_iterator timeGridEnd = m_TimeGrid.end();

  ModelResultType::iterator signalPos = signal.begin();

  for (TimeGridType::const_iterator gridPos = m_TimeGrid.begin(); gridPos != timeGridEnd; ++gridPos, ++signalPos)
  {
    *signalPos = ComputeSignalFromParameters(*gridPos, t, a1, a2, b1, b2);
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
    const auto t = parameters[POSITION_PARAMETER_t] ;
    const auto a1 = parameters[POSITION_PARAMETER_a1] ;
    const auto a2 = parameters[POSITION_PARAMETER_a2] ;
    const auto b1 = parameters[POSITION_PARAMETER_y1] ;
    const auto b2 = (a1 - a2) * t + b1;

    unsigned int timeSteps = m_TimeGrid.GetSize();

    const double taq = (m_TimeGrid.empty() == false) ? (m_TimeGrid.GetElement(timeSteps - 1)) : (mitkThrow() << "An exception occured because time grid is empty, method can't continue.");

    const double sfin = a2 * taq + b2;

    double smax = sfin;
    if ((a1 >= 0) && (a2 >= 0))
      smax = sfin;
    else if ((a1 < 0) && (a2 < 0))
      smax = b1;
    else if ((a1 > 0) && (a2 < 0))
      smax = (a1 * t + b1);
    else
    {
      if (abs(a1 * t) >= abs(a2 * (taq - t)))
        smax = b1;
      else smax = sfin;
    }

    double auc = 0.0;
    TimeGridType::const_iterator timeGridEnd = m_TimeGrid.end();
    for (TimeGridType::const_iterator gridPos = m_TimeGrid.begin(); gridPos != timeGridEnd -1; ++gridPos)
    {
      double currentGridPos = *gridPos;
      double nextGridPos = *(++gridPos);
      double deltaX = nextGridPos - currentGridPos;
      double deltaY = ComputeSignalFromParameters(nextGridPos, t, a1, a2, b1, b2) - ComputeSignalFromParameters(currentGridPos, t, a1, a2, b1, b2);
      double Yi = ComputeSignalFromParameters(currentGridPos, t, a1, a2, b1, b2);
      double intI = 0.5 * deltaX * deltaY + Yi * deltaX;
      auc += std::abs(intI);
      --gridPos;
    }

    DerivedParameterMapType result;

    result.insert(std::make_pair("AUC", auc));
    result.insert(std::make_pair("FinalUptake", sfin));
    result.insert(std::make_pair("Smax", smax));
    result.insert(std::make_pair("y-intercept2", b2));

    return result;
};

itk::LightObject::Pointer mitk::TwoStepLinearModel::InternalClone() const
{
  TwoStepLinearModel::Pointer newClone = TwoStepLinearModel::New();

  newClone->SetTimeGrid(this->m_TimeGrid);

  return newClone.GetPointer();
};
