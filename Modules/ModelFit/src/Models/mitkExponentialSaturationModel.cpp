/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkExponentialSaturationModel.h"
#include "mitkNumericConstants.h"

std::string mitk::ExponentialSaturationModel::GetModelDisplayName() const
{
  return "Exponential Saturation Model";
};

std::string mitk::ExponentialSaturationModel::GetModelType() const
{
  return "Generic";
};

mitk::ExponentialSaturationModel::FunctionStringType mitk::ExponentialSaturationModel::GetFunctionString() const
{
  return "if x<BAT f(x) = S0 , else f(x) = A * (1-exp(-*B(x-BAT)))";
};

std::string mitk::ExponentialSaturationModel::GetXName() const
{
  return "t";
};

mitk::ExponentialSaturationModel::ParameterNamesType
mitk::ExponentialSaturationModel::GetParameterNames() const
{
  ParameterNamesType result;
  result.push_back("bat");
  result.push_back("s0");
  result.push_back("sfin");
  result.push_back("rate");
  return result;
};

mitk::ExponentialSaturationModel::ParametersSizeType
mitk::ExponentialSaturationModel::GetNumberOfParameters() const
{
  return 4;
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
  return 0;
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
