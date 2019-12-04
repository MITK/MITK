/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMVConstrainedCostFunctionDecorator.h"

#include <iostream>

#include <mitkExceptionMacro.h>

mitk::MVConstrainedCostFunctionDecorator::MeasureType
  mitk::MVConstrainedCostFunctionDecorator::CalcMeasure(const ParametersType &parameters, const SignalType & /*signal*/) const
{
  if (m_ConstraintChecker.IsNull()) mitkThrow()<<"Error. Cannot calc measure. Constraint checker is not set";
  if (m_WrappedCostFunction.IsNull()) mitkThrow()<<"Error. Cannot calc measure. Wrapped metric is not set";

  m_EvaluationCount++;

  PenaltyValueType penalty = m_ConstraintChecker->GetPenaltySum(parameters);

  MeasureType measure;
  measure.SetSize(m_WrappedCostFunction->GetNumberOfValues());
  measure.Fill(penalty);

  if (penalty<m_FailureThreshold || !m_ActivateFailureThreshold)
  {
    MeasureType wrappedMeasure = m_WrappedCostFunction->GetValue(parameters);
    if (wrappedMeasure.Size() != measure.Size()) mitkThrow()<<"Error. Cannot calc measure. Penalty measure and wrapped measure have different size. Penalty size:"<<measure.Size()<<"; wrapped measure size: "<<wrappedMeasure.Size();

    for(unsigned int i=0; i<measure.GetSize(); ++i)
    {
      measure[i] += wrappedMeasure[i];
    }
    if (penalty > 0)
    {
      ++m_PenaltyCount;
    }
  }
  else
  {
    auto penalties = m_ConstraintChecker->GetPenalties(parameters);
    for (ParametersType::size_type pos = 0; pos < penalties.size(); ++pos)
    {
      if (penalties[pos] >= m_FailureThreshold)
      {
        m_LastFailedParameter = pos;
        break;
      }
    }
    m_FailureCount++;
  }

  return measure;
}

double
mitk::MVConstrainedCostFunctionDecorator::
GetPenaltyRatio() const
{
  return m_PenaltyCount / (double)m_EvaluationCount;
};

double
mitk::MVConstrainedCostFunctionDecorator::
GetFailureRatio() const
{
  return m_FailureCount / (double)m_EvaluationCount;
};

mitk::MVConstrainedCostFunctionDecorator::ParametersType::size_type
mitk::MVConstrainedCostFunctionDecorator::
GetFailedParameter() const
{
  return m_LastFailedParameter;
};
