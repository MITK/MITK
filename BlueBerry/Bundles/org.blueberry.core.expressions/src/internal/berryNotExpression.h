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
  static const std::size_t HASH_INITIAL;

  Expression::Pointer fExpression;

public:

  NotExpression(Expression::Pointer expression);

  EvaluationResult Evaluate(IEvaluationContext* context);

  void CollectExpressionInfo(ExpressionInfo* info);

  bool operator==(Expression& object);


  protected:

    std::size_t ComputeHashCode();
};

} // namespace berry

#endif // __BERRY_NOT_EXPRESSION_H__
