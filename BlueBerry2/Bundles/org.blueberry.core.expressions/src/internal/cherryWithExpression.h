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

#ifndef __CHERRY_WITH_EXPRESSION_H__
#define __CHERRY_WITH_EXPRESSION_H__

#include "cherryCompositeExpression.h"

#include "service/cherryIConfigurationElement.h"

#include "Poco/DOM/Element.h"

namespace cherry {

class WithExpression : public CompositeExpression {

private:
  std::string fVariable;
	static const std::string ATT_VARIABLE;

	/**
	 * The seed for the hash code for all with expressions.
	 */
	static const std::size_t HASH_INITIAL;

public:

  WithExpression(SmartPointer<IConfigurationElement> configElement);

	WithExpression(Poco::XML::Element* element);

	WithExpression(const std::string& variable);

	bool operator==(Expression& object);

  EvaluationResult Evaluate(IEvaluationContext::Pointer context);

  void CollectExpressionInfo(ExpressionInfo* info);


protected:

  std::size_t ComputeHashCode();

};

} // namespace cherry

#endif // __CHERRY_WITH_EXPRESSION_H__
