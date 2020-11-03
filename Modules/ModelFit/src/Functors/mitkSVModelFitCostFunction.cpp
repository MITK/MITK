/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSVModelFitCostFunction.h"
#include "mitkModelBase.h"

#include <iostream>


mitk::SVModelFitCostFunction::MeasureType mitk::SVModelFitCostFunction::GetValue(const ParametersType &parameter) const
{
  MeasureType measure;

  SignalType signal = m_Model->GetSignal(parameter);

  if(signal.GetSize() != m_Sample.GetSize()) itkExceptionMacro("Signal size does not matche sample size!");
  if(signal.GetSize() == 0)  itkExceptionMacro("Signal is empty!");

  measure = CalcMeasure(parameter, signal);

  return measure;
}

void mitk::SVModelFitCostFunction::GetDerivative (const ParametersType &parameters, DerivativeType &derivative) const
{
  ParametersType::SizeValueType paramCount = parameters.Size();

  derivative.SetSize(paramCount);

  for ( ParametersType::SizeValueType i = 0; i < paramCount; i++ )
  {
    ParametersType newParameters = parameters;
    newParameters[i] -= m_DerivativeStepLength;

    MeasureType e0 = GetValue(newParameters);

    newParameters = parameters;
    newParameters[i] += m_DerivativeStepLength;

    MeasureType e1 = GetValue(newParameters);

    derivative[i] = (e1 - e0) / ( 2 * m_DerivativeStepLength );
  }
};

unsigned int mitk::SVModelFitCostFunction::GetNumberOfParameters() const
{
  return m_Model->GetNumberOfParameters();
}

void mitk::SVModelFitCostFunction::SetSample(const SignalType &sampleSet)
{
  itkDebugMacro("setting Sample to " << sampleSet);
  this->m_Sample = sampleSet;
  this->Modified();
}

