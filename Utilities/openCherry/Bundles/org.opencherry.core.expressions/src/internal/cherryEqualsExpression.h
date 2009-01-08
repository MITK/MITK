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

#ifndef __CHERRY_EQUALS_EXPRESSION_H__
#define __CHERRY_EQUALS_EXPRESSION_H__

#include "../cherryExpression.h"
#include "../cherryExpressionInfo.h"
#include "../cherryIEvaluationContext.h"

#include "service/cherryIConfigurationElement.h"

#include "Poco/DOM/Element.h"

namespace cherry {

class EqualsExpression : public Expression {
	/**
	 * The seed for the hash code for all equals expressions.
	 */
private:

  static const intptr_t HASH_INITIAL;

	ExpressionVariable::Pointer fExpectedValue;

public:

  EqualsExpression(const ExpressionVariable::Pointer expectedValue);

	EqualsExpression(SmartPointer<IConfigurationElement> element);

	EqualsExpression(Poco::XML::Element* element);

	EvaluationResult Evaluate(IEvaluationContext* context);

	void CollectExpressionInfo(ExpressionInfo* info);

	bool operator==(Expression& object);

protected:

  intptr_t ComputeHashCode();
};

} // namespace cherry

#endif // __CHERRY_EQUALS_EXPRESSION_H__
