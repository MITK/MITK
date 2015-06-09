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
