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


#include "mitkAreaUnderTheCurveDescriptionParameter.h"

const std::string mitk::AreaUnderTheCurveDescriptionParameter::PARAMETER_NAME = "AreaUnderCurve";

mitk::AreaUnderTheCurveDescriptionParameter::AreaUnderTheCurveDescriptionParameter()
{
}

mitk::AreaUnderTheCurveDescriptionParameter::~AreaUnderTheCurveDescriptionParameter()
{
}

mitk::CurveDescriptionParameterBase::DescriptionParameterNamesType mitk::AreaUnderTheCurveDescriptionParameter::GetDescriptionParameterName() const
{
  DescriptionParameterNamesType result = { PARAMETER_NAME };
  return result;
}

mitk::CurveDescriptionParameterBase::DescriptionParameterResultsType mitk::AreaUnderTheCurveDescriptionParameter::ComputeCurveDescriptionParameter(const CurveType &curve, const CurveGridType &grid) const
{
  DescriptionParameterResultsType results;
  CurveDescriptionParameterResultType auc = 0;
  auto steps = grid.GetSize();

  for (CurveGridType::size_type i = 0; i < steps - 1; ++i)
  {
    double deltaX = grid(i + 1) - grid(i);
    double deltaY = curve(i + 1) - curve(i);
    double Yi = curve(i);

    double intI = 1 / 2 * deltaX * deltaY + Yi * deltaX;

    auc += std::abs(intI);

  }
  results.push_back(auc);
  return results;
}
