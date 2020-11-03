/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkModelFitFunctorBase.h"

mitk::ModelFitFunctorBase::OutputPixelArrayType
mitk::ModelFitFunctorBase::
Compute(const InputPixelArrayType& value, const ModelBase* model,
        const ModelBase::ParametersType& initialParameters) const
{
  if (!model)
  {
    itkExceptionMacro("Cannot compute fit. Passed model is not defined.");
  }

  if (model->GetNumberOfParameters() != initialParameters.Size())
  {
    itkExceptionMacro("Cannot compute fit. Parameter count of passed model and passed initial parameters differ. Model parameter count: "
                      << model->GetNumberOfParameters() << "; Initial parameters: " << initialParameters);
  }

  SignalType sample(value.size());

  for (SignalType::SizeValueType i = 0; i < sample.Size(); ++i)
  {
    sample[i] = value [i];
  }

  DebugParameterMapType debugParams;
  ParameterNamesType debugNames;
  if (this->m_DebugParameterMaps)
  {
    debugNames = this->GetDebugParameterNames();
  }

  ParametersType fittedParameters = DoModelFit(sample, model, initialParameters, debugParams);

  OutputPixelArrayType derivedParameters = this->GetDerivedParameters(model, fittedParameters);

  OutputPixelArrayType criteria = this->GetCriteria(model, fittedParameters, sample);

  OutputPixelArrayType evaluationParameters = this->GetEvaluationParameters(model, fittedParameters,
      sample);

  if (criteria.size() != this->GetCriterionNames().size())
  {
    itkExceptionMacro("ModelFitInfo implementation seems to be inconsitent. Number of criterion values is not equal to number of criterion names.");
  }

  OutputPixelArrayType result(fittedParameters.Size() + derivedParameters.size() + criteria.size() +
                              evaluationParameters.size() + debugNames.size());

  for (ParametersType::SizeValueType i = 0; i < fittedParameters.Size(); ++i)
  {
    result[i] = fittedParameters[i];
  }

  OutputPixelArrayType::size_type offset = fittedParameters.Size();

  for (OutputPixelArrayType::size_type j = 0; j < derivedParameters.size(); ++j)
  {
    result[offset + j] = derivedParameters[j];
  }

  offset += derivedParameters.size();
  for (OutputPixelArrayType::size_type j = 0; j < criteria.size(); ++j)
  {
    result[offset + j] = criteria[j];
  }

  offset += criteria.size();
  for (OutputPixelArrayType::size_type j = 0; j < evaluationParameters.size(); ++j)
  {
    result[offset + j] = evaluationParameters[j];
  }

  offset += evaluationParameters.size();
  for (OutputPixelArrayType::size_type j = 0; j < debugNames.size(); ++j)
  {
    DebugParameterMapType::const_iterator pos = debugParams.find(debugNames[j]);
    if (pos == debugParams.end())
    {
      itkExceptionMacro("ModelFitInfo implementation seems to be inconsitent. Debug parameter defined by functor is not in its returned debug map. Invalid debug parameter name: "<<debugNames[j]);
    }
    else
    {
      result[offset + j] = pos->second;
    }
  }

  return result;
};

unsigned int
mitk::ModelFitFunctorBase::GetNumberOfOutputs(const ModelBase* model) const
{
  if (!model)
  {
    itkExceptionMacro("Cannot get number of outputs. Model is not defined.");
  }

  return model->GetNumberOfParameters() + model->GetNumberOfDerivedParameters() +
         this->GetCriterionNames().size() + m_CostFunctionMap.size()+ this->GetDebugParameterNames().size();
};

void
mitk::ModelFitFunctorBase::ResetEvaluationParameters()
{
  m_Mutex.Lock();

  m_CostFunctionMap.clear();

  m_Mutex.Unlock();
};

void
mitk::ModelFitFunctorBase::RegisterEvaluationParameter(const std::string& parameterName,
    SVModelFitCostFunction* evaluationCostFunction)
{
  m_Mutex.Lock();

  SVModelFitCostFunction::Pointer costFunctPtr = evaluationCostFunction;

  m_CostFunctionMap.insert(std::make_pair(parameterName, costFunctPtr));

  m_Mutex.Unlock();
};

mitk::ModelFitFunctorBase::ParameterNamesType
mitk::ModelFitFunctorBase::GetEvaluationParameterNames() const
{
  m_Mutex.Lock();

  ParameterNamesType result;

  for (CostFunctionMapType::const_iterator pos = m_CostFunctionMap.begin();
       pos != m_CostFunctionMap.end(); ++pos)
  {
    result.push_back(pos->first);
  }

  m_Mutex.Unlock();

  return result;
};

const mitk::SVModelFitCostFunction*
mitk::ModelFitFunctorBase::GetEvaluationParameterCostFunction(const std::string& parameterName)
const
{
  const SVModelFitCostFunction* result = nullptr;

  m_Mutex.Lock();

  CostFunctionMapType::const_iterator pos = m_CostFunctionMap.find(parameterName);

  if (pos != m_CostFunctionMap.end())
  {
    result = (pos->second).GetPointer();
  }

  m_Mutex.Unlock();

  return result;
};

mitk::ModelFitFunctorBase::ParameterNamesType
mitk::ModelFitFunctorBase::GetDebugParameterNames() const
{
  ParameterNamesType result;

  if (this->m_DebugParameterMaps)
  {
    result = this->DefineDebugParameterNames();
  }

  return result;
};

mitk::ModelFitFunctorBase::
ModelFitFunctorBase() : m_DebugParameterMaps(false)
{};

mitk::ModelFitFunctorBase::
~ModelFitFunctorBase() {};

mitk::ModelFitFunctorBase::OutputPixelArrayType
mitk::ModelFitFunctorBase::GetDerivedParameters(const ModelBase* model,
    const ParametersType& parameters) const
{
  ModelBase::DerivedParameterMapType derivedParameterMap = model->GetDerivedParameters(parameters);
  OutputPixelArrayType result(derivedParameterMap.size());

  unsigned int i = 0;

  for (ModelBase::DerivedParameterMapType::const_iterator pos = derivedParameterMap.begin();
       pos != derivedParameterMap.end(); ++pos, ++i)
  {
    result[i] = pos->second;
  }

  return result;
};

mitk::ModelFitFunctorBase::OutputPixelArrayType
mitk::ModelFitFunctorBase::GetEvaluationParameters(const ModelBase* model,
    const ParametersType& parameters, const SignalType& sample) const
{
  m_Mutex.Lock();

  OutputPixelArrayType result(m_CostFunctionMap.size());

  unsigned int i = 0;

  for (CostFunctionMapType::const_iterator pos = m_CostFunctionMap.begin();
       pos != m_CostFunctionMap.end(); ++pos, ++i)
  {
    //break constness to configure evaluation cost functions. This operatoin is guarded be the mutex
    //after costFct->GetValue() the cost function may change its state again and is irrelevant for the
    //current call of GetEvaluationParameters
    SVModelFitCostFunction* costFct = const_cast<SVModelFitCostFunction*>(pos->second.GetPointer());

    costFct->SetModel(model);
    costFct->SetSample(sample);

    result[i] = costFct->GetValue(parameters);
  }

  m_Mutex.Unlock();

  return result;
};
