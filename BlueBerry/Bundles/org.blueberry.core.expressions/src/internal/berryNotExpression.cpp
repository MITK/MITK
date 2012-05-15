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

#include "berryNotExpression.h"

#include <Poco/Exception.h>
#include <Poco/Hash.h>

namespace berry {

const std::size_t NotExpression::HASH_INITIAL = Poco::hash("berry::NotExpression");

NotExpression::NotExpression(Expression::Pointer expression)
{
  poco_assert(expression.IsNotNull());

  fExpression= expression;
}

EvaluationResult
NotExpression::Evaluate(IEvaluationContext* context)
{
  return fExpression->Evaluate(context).Not();
}

void
NotExpression::CollectExpressionInfo(ExpressionInfo* info)
{
  fExpression->CollectExpressionInfo(info);
}

bool
NotExpression::operator==(Expression& object)
{
  try {
    NotExpression& that = dynamic_cast<NotExpression&>(object);
    return this->fExpression == that.fExpression;
  }
  catch (std::bad_cast)
  {
    return false;
  }
}

std::size_t
NotExpression::ComputeHashCode()
{
  return HASH_INITIAL * HASH_FACTOR + fExpression->HashCode();
}

}
