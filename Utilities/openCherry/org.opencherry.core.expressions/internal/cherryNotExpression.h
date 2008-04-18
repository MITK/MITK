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

#ifndef __CHERRY_NOT_EXPRESSION_H__
#define __CHERRY_NOT_EXPRESSION_H__

#include "../cherryExpression.h"

namespace cherry {

class NotExpression : public Expression {
	
private:
  
  /**
    * The seed for the hash code for all not expressions.
    */
  static const intptr_t HASH_INITIAL;

	Expression::Pointer fExpression;

public:
  
  NotExpression(Expression::Pointer expression);
	
	EvaluationResult Evaluate(IEvaluationContext* context);
	
	void CollectExpressionInfo(ExpressionInfo* info);

	bool operator==(Expression& object);

	
	protected:
	  
	  intptr_t ComputeHashCode();
};

} // namespace cherry

#endif // __CHERRY_NOT_EXPRESSION_H__
