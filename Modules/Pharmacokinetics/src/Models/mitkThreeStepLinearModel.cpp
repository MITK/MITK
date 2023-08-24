/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkThreeStepLinearModel.h"
#include <mitkIOUtil.h>


const std::string mitk::ThreeStepLinearModel::MODEL_DISPLAY_NAME = "Three Step Linear Model";

const std::string mitk::ThreeStepLinearModel::NAME_PARAMETER_s0 = "Baseline";
const std::string mitk::ThreeStepLinearModel::NAME_PARAMETER_t1 = "Time_point_1";
const std::string mitk::ThreeStepLinearModel::NAME_PARAMETER_t2 = "Time_point_2";
const std::string mitk::ThreeStepLinearModel::NAME_PARAMETER_a1 = "Slope_1";
const std::string mitk::ThreeStepLinearModel::NAME_PARAMETER_a2 = "Slope_2";

// Assuming that Model is calculated on Signal intensities I
const std::string mitk::ThreeStepLinearModel::UNIT_PARAMETER_s0 = "I";
const std::string mitk::ThreeStepLinearModel::UNIT_PARAMETER_t1 = "s";
const std::string mitk::ThreeStepLinearModel::UNIT_PARAMETER_t2 = "s";
const std::string mitk::ThreeStepLinearModel::UNIT_PARAMETER_a1 = "I/s";
const std::string mitk::ThreeStepLinearModel::UNIT_PARAMETER_a2 = "I/s";

const unsigned int mitk::ThreeStepLinearModel::POSITION_PARAMETER_s0 = 0;
const unsigned int mitk::ThreeStepLinearModel::POSITION_PARAMETER_t1 = 1;
const unsigned int mitk::ThreeStepLinearModel::POSITION_PARAMETER_t2 = 2;
const unsigned int mitk::ThreeStepLinearModel::POSITION_PARAMETER_a1 = 3;
const unsigned int mitk::ThreeStepLinearModel::POSITION_PARAMETER_a2 = 4;

const unsigned int mitk::ThreeStepLinearModel::NUMBER_OF_PARAMETERS = 5;

std::string mitk::ThreeStepLinearModel::GetModelDisplayName() const
{
  return MODEL_DISPLAY_NAME;
};

std::string mitk::ThreeStepLinearModel::GetModelType() const
{
  return "Generic";
};

mitk::ThreeStepLinearModel::FunctionStringType mitk::ThreeStepLinearModel::GetFunctionString() const
{
  return "Baseline if t<Time_point_1; Slope_1*t+Y_intercept_1 if Time_point_1<=t<=Time_point_2; Slope_2*t+Y_intercept_2 if Time_point_2<t";
};

std::string mitk::ThreeStepLinearModel::GetXName() const
{
  return "x";
};

mitk::ThreeStepLinearModel::ParameterNamesType
mitk::ThreeStepLinearModel::GetParameterNames() const
{
  ParameterNamesType result;
  result.push_back(NAME_PARAMETER_s0);
  result.push_back(NAME_PARAMETER_t1);
  result.push_back(NAME_PARAMETER_t2);
  result.push_back(NAME_PARAMETER_a1);
  result.push_back(NAME_PARAMETER_a2);


  return result;
};

mitk::ThreeStepLinearModel::ParametersSizeType
mitk::ThreeStepLinearModel::GetNumberOfParameters() const
{
  return NUMBER_OF_PARAMETERS;
};

mitk::ThreeStepLinearModel::ParamterUnitMapType
mitk::ThreeStepLinearModel::GetParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair(NAME_PARAMETER_s0, UNIT_PARAMETER_s0));
  result.insert(std::make_pair(NAME_PARAMETER_t1, UNIT_PARAMETER_t1));
  result.insert(std::make_pair(NAME_PARAMETER_t2, UNIT_PARAMETER_t2));
  result.insert(std::make_pair(NAME_PARAMETER_a1, UNIT_PARAMETER_a1));
  result.insert(std::make_pair(NAME_PARAMETER_a2, UNIT_PARAMETER_a2));

  return result;
};

mitk::ThreeStepLinearModel::ParameterNamesType
mitk::ThreeStepLinearModel::GetDerivedParameterNames() const
{
  ParameterNamesType result;
  result.push_back("AUC");
  result.push_back("FinalTimePoint");
  result.push_back("FinalUptake");
  result.push_back("Smax");
  result.push_back("y-intercept1");
  result.push_back("y-intercept2");

  return result;
};

mitk::ThreeStepLinearModel::ParametersSizeType
mitk::ThreeStepLinearModel::GetNumberOfDerivedParameters() const
{
  return 6;
};

double mitk::ThreeStepLinearModel::ComputeSignalFromParameters(double x, double s0, double t1, double t2, double a1, double a2, double b1, double b2)
{
  double signal = 0.0;

  if (x < t1)
  {
    signal = s0;
  }
  else if (x >= t1 && x <= t2)
  {
    signal = a1 * x + b1;
  }
  else
  {
    signal = a2 * x + b2;
  }


  return signal;
};

mitk::ThreeStepLinearModel::ParamterUnitMapType mitk::ThreeStepLinearModel::GetDerivedParameterUnits() const
{
  ParamterUnitMapType result;

  result.insert(std::make_pair("AUC", "I*s"));
  result.insert(std::make_pair("FinalTimePoint", "s"));
  result.insert(std::make_pair("FinalUptake", "I"));
  result.insert(std::make_pair("Smax", "I"));
  result.insert(std::make_pair("y-intercept1", "I"));
  result.insert(std::make_pair("y-intercept2", "I"));




  return result;
};

mitk::ThreeStepLinearModel::ModelResultType
mitk::ThreeStepLinearModel::ComputeModelfunction(const ParametersType& parameters) const
{
  //Model Parameters
  const double     s0 = (double) parameters[POSITION_PARAMETER_s0];
  const double     t1 = (double) parameters[POSITION_PARAMETER_t1] ;
  const double     t2 = (double) parameters[POSITION_PARAMETER_t2] ;
  const double     a1 = (double) parameters[POSITION_PARAMETER_a1] ;
  const double     a2 = (double) parameters[POSITION_PARAMETER_a2] ;

  double     b1 = s0 - a1 * t1;
  double     b2 = (a1 * t2 + b1) - (a2 * t2);

  ModelResultType signal(m_TimeGrid.GetSize());

  TimeGridType::const_iterator timeGridEnd = m_TimeGrid.end();

  ModelResultType::iterator signalPos = signal.begin();

  for (TimeGridType::const_iterator gridPos = m_TimeGrid.begin(); gridPos != timeGridEnd; ++gridPos, ++signalPos)
  {
    *signalPos = ComputeSignalFromParameters(*gridPos, s0, t1, t2, a1, a2, b1, b2);
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
    const double     s0 = (double) parameters[POSITION_PARAMETER_s0];
    const double     t1 = (double) parameters[POSITION_PARAMETER_t1] ;
    const double     t2 = (double) parameters[POSITION_PARAMETER_t2] ;
    const double     a1 = (double) parameters[POSITION_PARAMETER_a1] ;
    const double     a2 = (double) parameters[POSITION_PARAMETER_a2] ;

    const double     b1 = s0 - a1 * t1;
    const double     b2 = (a1 * t2 + b1) - (a2 * t2);

    unsigned int timeSteps = m_TimeGrid.GetSize();

    const double taq = (m_TimeGrid.empty() == false) ? (m_TimeGrid.GetElement(timeSteps - 1)) : ( mitkThrow() << "An exception occured because time grid is empty, method can't continue.");

    const double sfin = a2 * taq + b2;

    double smax = sfin;
    if ((a1 >= 0) && (a2 >= 0))
      smax = sfin;
    else if ((a1 < 0) && (a2 < 0))
      smax = s0;
    else if ((a1 > 0) && (a2 < 0))
      smax = (a1 * t2 + b1);
    else
    {
      if (abs(a1 * (t2 - t1)) >= abs(a2 * (taq - t2)))
        smax = s0;
      else smax = sfin;
    }

    double auc = 0.0;
    TimeGridType::const_iterator timeGridEnd = m_TimeGrid.end();

    for (TimeGridType::const_iterator gridPos = m_TimeGrid.begin(); gridPos != timeGridEnd - 1; ++gridPos)
    {
      double currentGridPos = *gridPos;
      double nextGridPos = *(++gridPos);
      double deltaX = nextGridPos - currentGridPos;
      double deltaY = ComputeSignalFromParameters(nextGridPos, s0, t1, t2, a1, a2, b1, b2) - ComputeSignalFromParameters(currentGridPos, s0, t1, t2, a1, a2, b1, b2);
      double Yi = ComputeSignalFromParameters(currentGridPos, s0, t1, t2, a1, a2, b1, b2 );
      double intI = 0.5 * deltaX * deltaY + Yi * deltaX;
      auc += std::abs(intI);
      --gridPos;
    }


    DerivedParameterMapType result;

    result.insert(std::make_pair("AUC", auc));
    result.insert(std::make_pair("FinalTimePoint", taq));
    result.insert(std::make_pair("FinalUptake", sfin));
    result.insert(std::make_pair("Smax", smax));
    result.insert(std::make_pair("y-intercept1", b1));
    result.insert(std::make_pair("y-intercept2", b2));


    return result;
};

itk::LightObject::Pointer mitk::ThreeStepLinearModel::InternalClone() const
{
  ThreeStepLinearModel::Pointer newClone = ThreeStepLinearModel::New();

  newClone->SetTimeGrid(this->m_TimeGrid);

  return newClone.GetPointer();
};
