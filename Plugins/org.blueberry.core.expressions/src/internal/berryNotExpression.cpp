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

#include "berryNotExpression.h"

#include <Poco/Exception.h>

namespace berry {

const uint NotExpression::HASH_INITIAL = qHash("berry::NotExpression");

NotExpression::NotExpression(Expression::Pointer expression)
{
  poco_assert(expression.IsNotNull());

  fExpression= expression;
}

EvaluationResult::ConstPointer
NotExpression::Evaluate(IEvaluationContext* context) const
{
  return fExpression->Evaluate(context)->Not();
}

void
NotExpression::CollectExpressionInfo(ExpressionInfo* info) const
{
  fExpression->CollectExpressionInfo(info);
}

bool
NotExpression::operator==(const Object* object) const
{
  if (const NotExpression* that = dynamic_cast<const NotExpression*>(object))
  {
    return this->fExpression == that->fExpression;
  }
  return false;
}

uint
NotExpression::ComputeHashCode() const
{
  return HASH_INITIAL * HASH_FACTOR + fExpression->HashCode();
}

}
