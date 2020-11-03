/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryCompositeExpression.h"

namespace berry
{

const uint CompositeExpression::HASH_INITIAL = qHash("berry::CompositeExpression");

void CompositeExpression::Add(Expression::Pointer expression)
{
  fExpressions.push_back(expression);
}

QList<Expression::Pointer> CompositeExpression::GetChildren()
{
  return fExpressions;
}

EvaluationResult::ConstPointer CompositeExpression::EvaluateAnd(IEvaluationContext* scope) const
{
  if (fExpressions.size() == 0)
    return EvaluationResult::TRUE_EVAL;
  EvaluationResult::ConstPointer result = EvaluationResult::TRUE_EVAL;
  foreach (Expression::Pointer iter, fExpressions)
  {
    result = result->And(iter->Evaluate(scope));
    // keep iterating even if we have a not loaded found. It can be
    // that we find a FALSE_EVAL which will result in a better result.
    if (result == EvaluationResult::FALSE_EVAL)
      return result;
  }
  return result;
}

EvaluationResult::ConstPointer CompositeExpression::EvaluateOr(IEvaluationContext* scope) const
{
  if (fExpressions.size() == 0)
    return EvaluationResult::TRUE_EVAL;
  EvaluationResult::ConstPointer result = EvaluationResult::FALSE_EVAL;
  foreach (Expression::Pointer iter, fExpressions)
  {
    result = result->Or(iter->Evaluate(scope));
    if (result == EvaluationResult::TRUE_EVAL)
      return result;
  }
  return result;
}

void CompositeExpression::CollectExpressionInfo(ExpressionInfo* info) const
{
  if (fExpressions.size() == 0)
    return;

  foreach (Expression::Pointer iter, fExpressions)
  {
    iter->CollectExpressionInfo(info);
  }
}

uint CompositeExpression::ComputeHashCode() const
{
  return HASH_INITIAL * HASH_FACTOR + this->HashCode(fExpressions);
}

} // namespace berry
