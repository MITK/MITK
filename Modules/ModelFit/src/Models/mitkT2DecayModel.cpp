/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkT2DecayModel.h"
#include "mitkNumericConstants.h"

std::string mitk::T2DecayModel::GetModelDisplayName() const
{
  return "T2 Decay Model";
};

std::string mitk::T2DecayModel::GetModelType() const
{
  return "MRSignal";
};

mitk::T2DecayModel::FunctionStringType mitk::T2DecayModel::GetFunctionString() const
{
  return "M0 * exp(-t/T2)";
};

std::string mitk::T2DecayModel::GetXName() const
{
  return "t";
};

mitk::T2DecayModel::ParameterNamesType
mitk::T2DecayModel::GetParameterNames() const
{
  ParameterNamesType result;
  result.push_back("M0");
  result.push_back("T2");
  return result;
};

mitk::T2DecayModel::ParametersSizeType
mitk::T2DecayModel::GetNumberOfParameters() const
{
  return 2;
};

mitk::T2DecayModel::ModelResultType
mitk::T2DecayModel::ComputeModelfunction(const ParametersType& parameters) const
{
  ModelResultType signal(m_TimeGrid.GetSize());

  ModelResultType::iterator signalPos = signal.begin();

  for (const auto& gridPos : m_TimeGrid)
  {
    *signalPos = parameters[0] * exp(-1.0 * gridPos/ parameters[1]);
    ++signalPos;
  }

  return signal;
};

mitk::T2DecayModel::ParameterNamesType mitk::T2DecayModel::GetStaticParameterNames() const
{
  ParameterNamesType result;

  return result;
}

mitk::T2DecayModel::ParametersSizeType  mitk::T2DecayModel::GetNumberOfStaticParameters() const
{
  return 0;
}

void mitk::T2DecayModel::SetStaticParameter(const ParameterNameType& /*name*/,
    const StaticParameterValuesType& /*values*/)
{
  //do nothing
};

mitk::T2DecayModel::StaticParameterValuesType mitk::T2DecayModel::GetStaticParameterValue(
  const ParameterNameType& /*name*/) const
{
  StaticParameterValuesType result;

  //do nothing

  return result;
};

mitk::T2DecayModel::DerivedParametersSizeType mitk::T2DecayModel::GetNumberOfDerivedParameters() const
{
  return 1;
}

mitk::T2DecayModel::DerivedParameterNamesType mitk::T2DecayModel::GetDerivedParameterNames() const
{
  ParameterNamesType result;
  result.push_back("R2");
  return result;
};

mitk::ModelBase::DerivedParameterMapType mitk::T2DecayModel::ComputeDerivedParameters(
  const mitk::ModelBase::ParametersType &parameters) const
{
  DerivedParameterMapType result;
  double inverse = 1.0 / (parameters[1] + mitk::eps);
  result.insert(std::make_pair("R2", inverse));
  return result;
};

itk::LightObject::Pointer mitk::T2DecayModel::InternalClone() const
{
  T2DecayModel::Pointer newClone = T2DecayModel::New();

  newClone->SetTimeGrid(this->m_TimeGrid);

  return newClone.GetPointer();
};
