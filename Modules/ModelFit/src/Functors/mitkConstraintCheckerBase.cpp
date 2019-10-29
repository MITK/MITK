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

#include "mitkConstraintCheckerBase.h"


mitk::ConstraintCheckerBase::PenaltyValueType
  mitk::ConstraintCheckerBase::GetPenaltySum(const ParametersType &parameters) const
{
  PenaltyValueType result = 0.0;
  PenaltyArrayType penalties = GetPenalties(parameters);

  for (PenaltyArrayType::iterator penaltyPos = penalties.begin(); penaltyPos != penalties.end(); ++penaltyPos)
  {
    result += *penaltyPos;
  }

  return result;
};
