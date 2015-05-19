/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
