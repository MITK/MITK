/*=========================================================================

Program:   BlueBerry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef BERRYCOMPOSITEEXPRESSION_H_
#define BERRYCOMPOSITEEXPRESSION_H_

#include "../berryExpression.h"
#include "../berryExpressionInfo.h"
#include "../berryEvaluationResult.h"
#include "../berryIEvaluationContext.h"

#include <vector>

namespace berry
{

class CompositeExpression : public Expression
{

public:
  osgiObjectMacro(CompositeExpression)

private:

  /**
   * The seed for the hash code for all composite expressions.
   */
  static const std::size_t HASH_INITIAL;

protected:
  std::vector<Expression::Pointer> fExpressions;

  virtual EvaluationResult EvaluateAnd(IEvaluationContext::Pointer scope);
  virtual EvaluationResult EvaluateOr(IEvaluationContext::Pointer scope);

  virtual std::size_t ComputeHashCode();

public:

  virtual void Add(Expression::Pointer expression);

  virtual void GetChildren(std::vector<Expression::Pointer>& children);

  virtual void CollectExpressionInfo(ExpressionInfo* info);

};

} // namespace berry

#endif /*BERRYCOMPOSITEEXPRESSION_H_*/
