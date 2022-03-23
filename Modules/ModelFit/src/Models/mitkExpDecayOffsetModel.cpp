/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkExpDecayOffsetModel.h"

std::string mitk::ExpDecayOffsetModel::GetModelDisplayName() const
{
  return "Exponential Decay Offset Model";
};

std::string mitk::ExpDecayOffsetModel::GetModelType() const
{
  return "Generic";
};

mitk::ExpDecayOffsetModel::FunctionStringType mitk::ExpDecayOffsetModel::GetFunctionString() const
{
  return "a*exp(-1.0*x*b)+c";
};

std::string mitk::ExpDecayOffsetModel::GetXName() const
{
  return "x";
};

mitk::ExpDecayOffsetModel::ParameterNamesType
mitk::ExpDecayOffsetModel::GetParameterNames() const
{
  ParameterNamesType result;
  result.push_back("a");
  result.push_back("b");
  result.push_back("c");
  return result;
};

mitk::ExpDecayOffsetModel::ParametersSizeType
mitk::ExpDecayOffsetModel::GetNumberOfParameters() const
{
  return 3;
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
  return 0;
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
