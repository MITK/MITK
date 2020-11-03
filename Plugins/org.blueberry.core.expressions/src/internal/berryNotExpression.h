/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __BERRY_NOT_EXPRESSION_H__
#define __BERRY_NOT_EXPRESSION_H__

#include "berryExpression.h"

namespace berry {

class NotExpression : public Expression {

private:

  /**
    * The seed for the hash code for all not expressions.
    */
  static const uint HASH_INITIAL;

  Expression::Pointer fExpression;

public:

  NotExpression(Expression::Pointer expression);

  EvaluationResult::ConstPointer Evaluate(IEvaluationContext* context) const override;

  void CollectExpressionInfo(ExpressionInfo* info) const override;

  bool operator==(const Object* object) const override;


  protected:

    uint ComputeHashCode() const override;
};

} // namespace berry

#endif // __BERRY_NOT_EXPRESSION_H__
