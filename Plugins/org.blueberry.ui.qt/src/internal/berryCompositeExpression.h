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
  berryObjectMacro(CompositeExpression)

private:

  /**
   * The seed for the hash code for all composite expressions.
   */
  static const uint HASH_INITIAL;

protected:
  QList<Expression::Pointer> fExpressions;

  virtual EvaluationResult::ConstPointer EvaluateAnd(IEvaluationContext* scope) const;
  virtual EvaluationResult::ConstPointer EvaluateOr(IEvaluationContext* scope) const;

  virtual uint ComputeHashCode() const override;

public:

  virtual void Add(Expression::Pointer expression);

  virtual QList<Expression::Pointer> GetChildren();

  virtual void CollectExpressionInfo(ExpressionInfo* info) const override;

};

} // namespace berry

#endif /*BERRYCOMPOSITEEXPRESSION_H_*/
