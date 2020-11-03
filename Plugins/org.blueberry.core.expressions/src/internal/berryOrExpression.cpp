/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryOrExpression.h"

namespace berry {

EvaluationResult::ConstPointer OrExpression::Evaluate(IEvaluationContext* context) const
{
  return this->EvaluateOr(context);
}

bool OrExpression::operator==(const Object* object) const
{
  if(const OrExpression* that = dynamic_cast<const OrExpression*>(object))
  {
    return this->fExpressions == that->fExpressions;
  }
  return false;
}

}
