/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestModel.h"

mitk::TestModel::ParamterScaleMapType
mitk::TestModel::GetParameterScales() const
{
  ParamterScaleMapType result;
  ParameterNamesType names = GetParameterNames();

  for (ParameterNamesType::iterator pos = names.begin(); pos != names.end(); ++pos)
  {
    result.insert(std::make_pair(*pos, 2.0));
  }

  return result;
};

mitk::TestModel::ParamterUnitMapType
mitk::TestModel::GetParameterUnits() const
{
  ParamterUnitMapType result;
  ParameterNamesType names = GetParameterNames();

  for (ParameterNamesType::iterator pos = names.begin(); pos != names.end(); ++pos)
  {
    result.insert(std::make_pair(*pos, "ParamUnit"));
  }

  return result;
};

std::string mitk::TestModel::GetModelDisplayName() const
{
  return "Test Model 1";
};

std::string mitk::TestModel::GetModelType() const
{
  return "TestModels";
};

mitk::TestModel::FunctionStringType mitk::TestModel::GetFunctionString() const
{
  return "slope*X+offset";
};

std::string mitk::TestModel::GetXName() const
{
  return "X";
};

std::string mitk::TestModel::GetXAxisName() const
{
  return "xAxis";
};

std::string mitk::TestModel::GetXAxisUnit() const
{
  return "mm";
};

std::string mitk::TestModel::GetYAxisName() const
{
  return "yAxis";
};

std::string mitk::TestModel::GetYAxisUnit() const
{
  return "s";
}

mitk::TestModel::ParameterNamesType
mitk::TestModel::GetParameterNames() const
{
  ParameterNamesType result;
  result.push_back("slope");
  result.push_back("offset");
  return result;
};

mitk::TestModel::ParametersSizeType
mitk::TestModel::GetNumberOfParameters() const
{
  return 2;
};

mitk::TestModel::ParameterNamesType
mitk::TestModel::GetDerivedParameterNames() const
{
  ParameterNamesType result;
  result.push_back("x-intercept");
  return result;
};

mitk::TestModel::ParametersSizeType
mitk::TestModel::GetNumberOfDerivedParameters() const
{
  return 1;
};

mitk::TestModel::ModelResultType
mitk::TestModel::ComputeModelfunction(const ParametersType& parameters) const
{
  ModelResultType signal(m_TimeGrid.GetSize());

  TimeGridType::const_iterator timeGridEnd = m_TimeGrid.end();
  ModelResultType::iterator signalPos = signal.begin();

  for (TimeGridType::const_iterator gridPos = m_TimeGrid.begin(); gridPos != timeGridEnd;
       ++gridPos, ++signalPos)
  {
    *signalPos = parameters[0] * (*gridPos) + parameters[1];
  }

  return signal;
};

mitk::TestModel::ParameterNamesType mitk::TestModel::GetStaticParameterNames() const
{
  ParameterNamesType result;

  return result;
}

mitk::TestModel::ParametersSizeType  mitk::TestModel::GetNumberOfStaticParameters() const
{
  return 0;
}

void mitk::TestModel::SetStaticParameter(const ParameterNameType& /*name*/,
    const StaticParameterValuesType& /*values*/)
{
  //do nothing
};

mitk::TestModel::StaticParameterValuesType mitk::TestModel::GetStaticParameterValue(
  const ParameterNameType& /*name*/) const
{
  StaticParameterValuesType result;

  //do nothing

  return result;
};

mitk::ModelBase::DerivedParameterMapType mitk::TestModel::ComputeDerivedParameters(
  const mitk::ModelBase::ParametersType& parameters) const
{
  DerivedParameterMapType result;
  double intercept = -1 * parameters[1] / parameters[0];
  result.insert(std::make_pair("x-intercept", intercept));
  return result;
};

itk::LightObject::Pointer mitk::TestModel::InternalClone() const
{
  TestModel::Pointer newClone = TestModel::New();

  newClone->SetTimeGrid(this->m_TimeGrid);

  return newClone.GetPointer();
};
