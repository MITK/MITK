/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkLinearModel.h"

std::string mitk::LinearModel::GetModelDisplayName() const
{
  return "Linear Model";
};

std::string mitk::LinearModel::GetModelType() const
{
  return "Generic";
};

mitk::LinearModel::FunctionStringType mitk::LinearModel::GetFunctionString() const
{
  return "slope*x+offset";
};

std::string mitk::LinearModel::GetXName() const
{
  return "x";
};

mitk::LinearModel::ParameterNamesType
mitk::LinearModel::GetParameterNames() const
{
  ParameterNamesType result;
  result.push_back("slope");
  result.push_back("offset");
  return result;
};

mitk::LinearModel::ParametersSizeType
mitk::LinearModel::GetNumberOfParameters() const
{
  return 2;
};

mitk::LinearModel::ParameterNamesType
mitk::LinearModel::GetDerivedParameterNames() const
{
  ParameterNamesType result;
  result.push_back("x-intercept");
  return result;
};

mitk::LinearModel::ParametersSizeType
mitk::LinearModel::GetNumberOfDerivedParameters() const
{
  return 1;
};

mitk::LinearModel::ModelResultType
mitk::LinearModel::ComputeModelfunction(const ParametersType& parameters) const
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

mitk::LinearModel::ParameterNamesType mitk::LinearModel::GetStaticParameterNames() const
{
  ParameterNamesType result;

  return result;
}

mitk::LinearModel::ParametersSizeType  mitk::LinearModel::GetNumberOfStaticParameters() const
{
  return 0;
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
  double intercept = -1 * parameters[1] / parameters[0];
  result.insert(std::make_pair("x-intercept", intercept));
  return result;
};

itk::LightObject::Pointer mitk::LinearModel::InternalClone() const
{
  LinearModel::Pointer newClone = LinearModel::New();

  newClone->SetTimeGrid(this->m_TimeGrid);

  return newClone.GetPointer();
};
