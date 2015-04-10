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

  EvaluationResult::ConstPointer Evaluate(IEvaluationContext* context) const;

  void CollectExpressionInfo(ExpressionInfo* info) const;

  bool operator==(const Object* object) const;


  protected:

    uint ComputeHashCode() const;
};

} // namespace berry

#endif // __BERRY_NOT_EXPRESSION_H__
