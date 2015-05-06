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

#ifndef __BERRY_EQUALS_EXPRESSION_H__
#define __BERRY_EQUALS_EXPRESSION_H__

#include "berryExpression.h"
#include "berryExpressionInfo.h"
#include "berryIEvaluationContext.h"

#include "Poco/DOM/Element.h"

namespace berry {

struct IConfigurationElement;

class EqualsExpression : public Expression {
  /**
   * The seed for the hash code for all equals expressions.
   */
private:

  static const uint HASH_INITIAL;

  Object::Pointer fExpectedValue;

public:

  EqualsExpression(const Object::Pointer expectedValue);

  EqualsExpression(const SmartPointer<IConfigurationElement>& element);

  EqualsExpression(Poco::XML::Element* element);

  EvaluationResult::ConstPointer Evaluate(IEvaluationContext* context) const;

  void CollectExpressionInfo(ExpressionInfo* info) const;

  bool operator==(const Object* object) const;

protected:

  uint ComputeHashCode() const;
};

} // namespace berry

#endif // __BERRY_EQUALS_EXPRESSION_H__
