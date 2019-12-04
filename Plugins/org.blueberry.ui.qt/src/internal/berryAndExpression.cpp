/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryAndExpression.h"

namespace berry
{

bool AndExpression::operator==(const Object* object) const
{
  if(const AndExpression* that = dynamic_cast<const AndExpression*>(object))
  {
    return this->Equals(this->fExpressions, that->fExpressions);
  }
  return false;
}

EvaluationResult::ConstPointer
AndExpression::Evaluate(IEvaluationContext* context) const
{
  return this->EvaluateAnd(context);
}

} // namespace berry
