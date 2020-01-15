/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __BERRY_WITH_EXPRESSION_H__
#define __BERRY_WITH_EXPRESSION_H__

#include "berryCompositeExpression.h"

#include "Poco/DOM/Element.h"

namespace berry {

struct IConfigurationElement;

class WithExpression : public CompositeExpression {

private:
  QString fVariable;
  static const QString ATT_VARIABLE;

  /**
   * The seed for the hash code for all with expressions.
   */
  static const uint HASH_INITIAL;

public:

  WithExpression(const SmartPointer<IConfigurationElement>& configElement);

  WithExpression(Poco::XML::Element* element);

  WithExpression(const QString &variable);

  bool operator==(const Object* object) const override;

  EvaluationResult::ConstPointer Evaluate(IEvaluationContext* context) const override;

  void CollectExpressionInfo(ExpressionInfo* info) const override;


protected:

  uint ComputeHashCode() const override;

};

} // namespace berry

#endif // __BERRY_WITH_EXPRESSION_H__
