/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkTimeToPeakCurveDescriptionParameter.h"

const std::string mitk::TimeToPeakCurveDescriptionParameter::PARAMETER_PEAK_NAME = "Peak";
const std::string mitk::TimeToPeakCurveDescriptionParameter::PARAMETER_TIME_NAME = "Time";

mitk::TimeToPeakCurveDescriptionParameter::TimeToPeakCurveDescriptionParameter()
{
}

mitk::TimeToPeakCurveDescriptionParameter::~TimeToPeakCurveDescriptionParameter()
{
}

mitk::CurveDescriptionParameterBase::DescriptionParameterNamesType mitk::TimeToPeakCurveDescriptionParameter::GetDescriptionParameterName() const
{
  DescriptionParameterNamesType result = { PARAMETER_TIME_NAME, PARAMETER_PEAK_NAME };
  return result;
}

mitk::CurveDescriptionParameterBase::DescriptionParameterResultsType mitk::TimeToPeakCurveDescriptionParameter::ComputeCurveDescriptionParameter(const CurveType &curve, const CurveGridType &grid) const
{
  DescriptionParameterResultsType result;
  CurveDescriptionParameterResultType time = itk::NumericTraits<CurveDescriptionParameterResultType>::NonpositiveMin();
  CurveDescriptionParameterResultType maximum = itk::NumericTraits<CurveDescriptionParameterResultType>::NonpositiveMin();
  int steps = grid.GetSize();

  for (int i = 0; i < steps - 1; ++i)
  {
    if (curve(i) > maximum)
    {
      maximum = curve(i);
      time = grid(i);
    }
  }

  result.push_back(time);
  result.push_back(maximum);
  return result;
}
