/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkLevenbergMarquardtModelFitFunctor.h"

#include "mitkSquaredDifferencesFitCostFunction.h"
#include "mitkSumOfSquaredDifferencesFitCostFunction.h"
#include <chrono>
#include <mitkExceptionMacro.h>

mitk::LevenbergMarquardtModelFitFunctor::
LevenbergMarquardtModelFitFunctor(): m_Epsilon(1e-5), m_GradientTolerance(1e-3),
  m_ValueTolerance(1e-5), m_Iterations(1000), m_DerivativeStepLength(1e-5),
  m_ActivateFailureThreshold(true)
{};

mitk::LevenbergMarquardtModelFitFunctor::
~LevenbergMarquardtModelFitFunctor()
{};

mitk::LevenbergMarquardtModelFitFunctor::ParameterNamesType
mitk::LevenbergMarquardtModelFitFunctor::
GetCriterionNames() const
{
  ParameterNamesType names;
  names.push_back("sum_diff^2");
  return names;
};

mitk::LevenbergMarquardtModelFitFunctor::OutputPixelArrayType
mitk::LevenbergMarquardtModelFitFunctor::
GetCriteria(const ModelBase* model, const ParametersType& parameters,
              const SignalType& sample) const
{
  ::mitk::SumOfSquaredDifferencesFitCostFunction::Pointer metric
    = ::mitk::SumOfSquaredDifferencesFitCostFunction::New();
  metric->SetModel(model);
  metric->SetSample(sample);

  mitk::LevenbergMarquardtModelFitFunctor::OutputPixelArrayType result(1);
  result[0] = metric->GetValue(parameters);

  return result;
};

mitk::MVModelFitCostFunction::Pointer mitk::LevenbergMarquardtModelFitFunctor::GenerateCostFunction(
  const SignalType& value, const ModelBase* model) const
{
  ::mitk::SquaredDifferencesFitCostFunction::Pointer metric
    = ::mitk::SquaredDifferencesFitCostFunction::New();
  metric->SetModel(model);
  metric->SetSample(value);
  metric->SetDerivativeStepLength(m_DerivativeStepLength);

  mitk::MVModelFitCostFunction::Pointer result = metric.GetPointer();

  if (m_ConstraintChecker.IsNotNull())
  {
    ::mitk::MVConstrainedCostFunctionDecorator::Pointer decorator
      = ::mitk::MVConstrainedCostFunctionDecorator::New();
    decorator->SetConstraintChecker(m_ConstraintChecker);
    decorator->SetWrappedCostFunction(metric);
    decorator->SetFailureThreshold(m_ConstraintChecker->GetFailedConstraintValue());

    decorator->SetModel(model);
    decorator->SetSample(value);
    decorator->SetActivateFailureThreshold(m_ActivateFailureThreshold);
    result = decorator;
  }

  return result;
};

mitk::LevenbergMarquardtModelFitFunctor::ParameterNamesType
mitk::LevenbergMarquardtModelFitFunctor::DefineDebugParameterNames() const
{
  ParameterNamesType result;
  result.push_back("optimization_time");
  result.push_back("nr_of_iterations");
  result.push_back("stop_condition");
  if (m_ConstraintChecker.IsNotNull())
  {
    result.push_back("constraint_penalty_ratio");
    result.push_back("constraint_failure_ratio");
    result.push_back("constraint_last_failed_parameter");
  }
  return result;
};

mitk::LevenbergMarquardtModelFitFunctor::ParametersType
mitk::LevenbergMarquardtModelFitFunctor::
DoModelFit(const SignalType& value, const ModelBase* model,
           const ModelBase::ParametersType& initialParameters,
           DebugParameterMapType& debugParameters) const
{
    std::chrono::time_point<std::chrono::system_clock> startTime;
    startTime = std::chrono::system_clock::now();
  ::itk::LevenbergMarquardtOptimizer::ParametersType internalInitParam = initialParameters;
  ::itk::LevenbergMarquardtOptimizer::ScalesType scales = m_Scales;

  if (initialParameters.GetNumberOfElements() != model->GetNumberOfParameters())
  {
    MITK_DEBUG <<
               "Size of initial parameters of fit functor optimizer do not match number of model parameters. Renitialize parameters with 0.0.";
    internalInitParam.SetSize(model->GetNumberOfParameters());
    internalInitParam.Fill(0.0);
  }

  if (m_Scales.GetNumberOfElements() != model->GetNumberOfParameters())
  {
    MITK_DEBUG <<
               "Size of scales of fit functor optimizer do not match number of model parameters. Reinitialize scales with 1.0.";
    scales.SetSize(model->GetNumberOfParameters());
    scales.Fill(1.0);
  }

  mitk::MVModelFitCostFunction::Pointer metric = this->GenerateCostFunction(value, model);

  ::itk::LevenbergMarquardtOptimizer::Pointer optimizer = ::itk::LevenbergMarquardtOptimizer::New();

  optimizer->SetCostFunction(metric);
  optimizer->SetEpsilonFunction(m_Epsilon);
  optimizer->SetGradientTolerance(m_GradientTolerance);
  optimizer->SetNumberOfIterations(m_Iterations);
  optimizer->SetScales(scales);
  optimizer->SetInitialPosition(internalInitParam);

  optimizer->StartOptimization();

  itk::Optimizer::ParametersType position = optimizer->GetCurrentPosition();

  std::chrono::time_point<std::chrono::system_clock> stopTime;
  stopTime = std::chrono::system_clock::now();
  debugParameters.clear();
  if (this->GetDebugParameterMaps())
  {
    const auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime).count();
    debugParameters.insert(std::make_pair("optimization_time", timeDiff));

    ParameterImagePixelType value = optimizer->GetOptimizer()->get_num_iterations();
    debugParameters.insert(std::make_pair("nr_of_iterations", value));
    value = optimizer->GetOptimizer()->get_failure_code();
    debugParameters.insert(std::make_pair("stop_condition", value));


    const ::mitk::MVConstrainedCostFunctionDecorator* decorator = dynamic_cast<const ::mitk::MVConstrainedCostFunctionDecorator*>(metric.GetPointer());
    if (decorator)
    {
      value = decorator->GetPenaltyRatio();
      debugParameters.insert(std::make_pair("constraint_penalty_ratio", value));
      value = decorator->GetFailureRatio();
      debugParameters.insert(std::make_pair("constraint_failure_ratio", value));
      value = decorator->GetFailedParameter();
      debugParameters.insert(std::make_pair("constraint_last_failed_parameter", value));
    }
    else
    {
      if (m_ConstraintChecker.IsNotNull())
      {
        mitkThrow() << "Fit functor has invalid state/wrong implementation. Constraint checker is set, but used metric seems to be no MVContstrainedCostFunctionDecorator.";
      }
    }
  }

  return position;
};
