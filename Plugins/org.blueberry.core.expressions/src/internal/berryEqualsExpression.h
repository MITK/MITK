/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

  EvaluationResult::ConstPointer Evaluate(IEvaluationContext* context) const override;

  void CollectExpressionInfo(ExpressionInfo* info) const override;

  bool operator==(const Object* object) const override;

protected:

  uint ComputeHashCode() const override;
};

} // namespace berry

#endif // __BERRY_EQUALS_EXPRESSION_H__
