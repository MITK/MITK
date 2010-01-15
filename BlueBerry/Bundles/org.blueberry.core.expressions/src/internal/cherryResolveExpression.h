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

#ifndef __CHERRY_RESOLVE_EXPRESSION_H__
#define __CHERRY_RESOLVE_EXPRESSION_H__

#include "cherryCompositeExpression.h"

#include "service/cherryIConfigurationElement.h"

#include "Poco/DOM/Element.h"

#include <vector>

namespace cherry {

class ResolveExpression : public CompositeExpression {

private:
  std::string fVariable;
	std::vector<Object::Pointer> fArgs;

	static const std::string ATT_VARIABLE;
	static const std::string ATT_ARGS;

	/**
	 * The seed for the hash code for all resolve expressions.
	 */
	static const std::size_t HASH_INITIAL;

public:

  ResolveExpression(SmartPointer<IConfigurationElement> configElement);

	ResolveExpression(Poco::XML::Element* element);

	ResolveExpression(const std::string& variable, std::vector<Object::Pointer>& args);

	EvaluationResult Evaluate(IEvaluationContext* context);

	void CollectExpressionInfo(ExpressionInfo* info);

	bool operator==(Expression& object);


protected:

  std::size_t ComputeHashCode();
};

} // namespace cherry

#endif // __CHERRY_RESOLVE_EXPRESSION_H__
