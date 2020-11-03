/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "mitkNormalizedSumOfSquaredDifferencesFitCostFunction.h"

mitk::NormalizedSumOfSquaredDifferencesFitCostFunction::MeasureType mitk::NormalizedSumOfSquaredDifferencesFitCostFunction::CalcMeasure(const ParametersType &/*parameters*/, const SignalType &signal) const
{
  MeasureType measure = 0.0;
  unsigned int signalSize = signal.GetSize();

  for(SignalType::size_type i=0; i<signal.GetSize(); ++i)
  {
    measure += (m_Sample[i] - signal[i]) *  (m_Sample[i] - signal[i]);
  }
  measure = measure / signalSize;

  return measure;
}
