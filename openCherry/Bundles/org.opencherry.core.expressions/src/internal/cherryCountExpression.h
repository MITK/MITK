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

#ifndef __CHERRY_COUNT_EXPRESSION_H__
#define __CHERRY_COUNT_EXPRESSION_H__

#include "../cherryExpression.h"
#include "../cherryIEvaluationContext.h"
#include "../cherryExpressionInfo.h"

#include "service/cherryIConfigurationElement.h"

#include "Poco/DOM/Element.h"

#include <string>

namespace cherry {

class CountExpression : public Expression {

private:
  static const int ANY_NUMBER;
	static const int EXACT;
	 static const int ONE_OR_MORE;
	 static const int NONE_OR_ONE;
	 static const int NONE;
	 static const int UNKNOWN;

	/**
	 * The seed for the hash code for all count expressions.
	 */
	static const std::size_t HASH_INITIAL;

	 int fMode;
	 int fSize;

 void InitializeSize(std::string size);

public:
  CountExpression(SmartPointer<IConfigurationElement> configElement);

	 CountExpression(Poco::XML::Element* element);

	 CountExpression(const std::string& size);

	 EvaluationResult Evaluate(IEvaluationContext* context);

	 void CollectExpressionInfo(ExpressionInfo* info);

	 bool operator==(Expression& object);


protected:

  std::size_t ComputeHashCode();

};

} // namespace cherry

#endif // __CHERRY_COUNT_EXPRESSION_H__
