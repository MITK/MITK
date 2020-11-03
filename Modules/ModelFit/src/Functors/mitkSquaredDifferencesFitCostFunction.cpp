/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSquaredDifferencesFitCostFunction.h"

mitk::SquaredDifferencesFitCostFunction::MeasureType mitk::SquaredDifferencesFitCostFunction::CalcMeasure(const ParametersType &/*parameters*/, const SignalType &signal) const
{
  MeasureType measure;
  measure.SetSize(signal.GetSize());

  for(SignalType::size_type i=0; i<signal.GetSize(); ++i)
  {
    measure[i] = (m_Sample[i] - signal[i]) *  (m_Sample[i] - signal[i]);
  }

  return measure;
}
