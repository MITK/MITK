/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
