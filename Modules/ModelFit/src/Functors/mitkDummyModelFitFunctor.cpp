/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDummyModelFitFunctor.h"

#include "mitkSquaredDifferencesFitCostFunction.h"
#include "mitkSumOfSquaredDifferencesFitCostFunction.h"
#include <chrono>
#include <mitkExceptionMacro.h>

mitk::DummyModelFitFunctor::
DummyModelFitFunctor(): m_DerivativeStepLength(1e-5)
{};

mitk::DummyModelFitFunctor::
~DummyModelFitFunctor()
{};

mitk::DummyModelFitFunctor::ParameterNamesType
mitk::DummyModelFitFunctor::
GetCriterionNames() const
{
  ParameterNamesType names;
  names.push_back("sum_diff^2");
  return names;
};

mitk::DummyModelFitFunctor::OutputPixelArrayType
mitk::DummyModelFitFunctor::
GetCriteria(const ModelBase* model, const ParametersType& parameters,
              const SignalType& sample) const
{
  ::mitk::SumOfSquaredDifferencesFitCostFunction::Pointer metric
    = ::mitk::SumOfSquaredDifferencesFitCostFunction::New();
  metric->SetModel(model);
  metric->SetSample(sample);

  mitk::DummyModelFitFunctor::OutputPixelArrayType result(1);
  result[0] = metric->GetValue(parameters);

  return result;
};

mitk::MVModelFitCostFunction::Pointer mitk::DummyModelFitFunctor::GenerateCostFunction(
  const SignalType& value, const ModelBase* model) const
{
  ::mitk::SquaredDifferencesFitCostFunction::Pointer metric
    = ::mitk::SquaredDifferencesFitCostFunction::New();
  metric->SetModel(model);
  metric->SetSample(value);
  metric->SetDerivativeStepLength(m_DerivativeStepLength);

  mitk::MVModelFitCostFunction::Pointer result = metric.GetPointer();

  return result;
};

mitk::DummyModelFitFunctor::ParametersType
mitk::DummyModelFitFunctor::
DoModelFit(const SignalType& value, const ModelBase* model,
           const ModelBase::ParametersType& initialParameters,
           DebugParameterMapType& /*debugParameters*/) const
{
  mitk::MVModelFitCostFunction::Pointer metric = this->GenerateCostFunction(value, model);

  MITK_INFO << "DEBUG signal: " << value;
  MITK_INFO << "DEBUG time grid: " << model->GetTimeGrid();
  MITK_INFO << "DEBUG model: " << model->GetSignal(initialParameters);
  MITK_INFO << "DEBUG parameters: " << initialParameters;

  MITK_INFO << "DEBUG metric values: " << metric->GetValue(initialParameters);

  return initialParameters;
};

mitk::DummyModelFitFunctor::ParameterNamesType
mitk::DummyModelFitFunctor::DefineDebugParameterNames() const
{
  return mitk::DummyModelFitFunctor::ParameterNamesType();
};
