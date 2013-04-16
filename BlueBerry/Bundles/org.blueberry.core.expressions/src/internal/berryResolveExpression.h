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

#ifndef __BERRY_RESOLVE_EXPRESSION_H__
#define __BERRY_RESOLVE_EXPRESSION_H__

#include "berryCompositeExpression.h"

#include "service/berryIConfigurationElement.h"

#include "Poco/DOM/Element.h"

#include <vector>

namespace berry {

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

} // namespace berry

#endif // __BERRY_RESOLVE_EXPRESSION_H__
