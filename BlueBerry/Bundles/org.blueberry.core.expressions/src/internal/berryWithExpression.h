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

#ifndef __BERRY_WITH_EXPRESSION_H__
#define __BERRY_WITH_EXPRESSION_H__

#include "berryCompositeExpression.h"

#include "service/berryIConfigurationElement.h"

#include "Poco/DOM/Element.h"

namespace berry {

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

  EvaluationResult Evaluate(IEvaluationContext* context);

  void CollectExpressionInfo(ExpressionInfo* info);


protected:

  std::size_t ComputeHashCode();

};

} // namespace berry

#endif // __BERRY_WITH_EXPRESSION_H__
