/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYCOMPOSITEEXPRESSION_H_
#define BERRYCOMPOSITEEXPRESSION_H_

#include "berryExpression.h"
#include "berryExpressionInfo.h"
#include "berryEvaluationResult.h"
#include "berryIEvaluationContext.h"


namespace berry
{

class CompositeExpression : public Expression
{

public:
  berryObjectMacro(CompositeExpression);

private:

  /**
   * The seed for the hash code for all composite expressions.
   */
  static const uint HASH_INITIAL;

protected:
  QList<Expression::Pointer> fExpressions;

  virtual EvaluationResult::ConstPointer EvaluateAnd(IEvaluationContext* scope) const;
  virtual EvaluationResult::ConstPointer EvaluateOr(IEvaluationContext* scope) const;

  uint ComputeHashCode() const override;

public:

  virtual void Add(Expression::Pointer expression);

  virtual QList<Expression::Pointer> GetChildren();

  void CollectExpressionInfo(ExpressionInfo* info) const override;

};

} // namespace berry

#endif /*BERRYCOMPOSITEEXPRESSION_H_*/
