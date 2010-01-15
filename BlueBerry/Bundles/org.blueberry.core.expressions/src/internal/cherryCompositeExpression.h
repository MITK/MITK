/*=========================================================================
 
Program:   openCherry Platform
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

#ifndef CHERRYCOMPOSITEEXPRESSION_H_
#define CHERRYCOMPOSITEEXPRESSION_H_

#include "../cherryExpression.h"
#include "../cherryExpressionInfo.h"
#include "../cherryEvaluationResult.h"
#include "../cherryIEvaluationContext.h"

#include <vector>

namespace cherry
{

class CompositeExpression : public Expression
{

public:
  cherryObjectMacro(CompositeExpression)
  
private:

  /**
   * The seed for the hash code for all composite expressions.
   */
  static const std::size_t HASH_INITIAL;

protected:
  std::vector<Expression::Pointer> fExpressions;

  virtual EvaluationResult EvaluateAnd(IEvaluationContext* scope);
  virtual EvaluationResult EvaluateOr(IEvaluationContext* scope);

  virtual std::size_t ComputeHashCode();

public:

  virtual void Add(Expression::Pointer expression);

  virtual void GetChildren(std::vector<Expression::Pointer>& children);

  virtual void CollectExpressionInfo(ExpressionInfo* info);

};

} // namespace cherry

#endif /*CHERRYCOMPOSITEEXPRESSION_H_*/
