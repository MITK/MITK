/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __BERRY_RESOLVE_EXPRESSION_H__
#define __BERRY_RESOLVE_EXPRESSION_H__

#include "berryCompositeExpression.h"

#include "Poco/DOM/Element.h"

#include <vector>

namespace berry {

struct IConfigurationElement;

class ResolveExpression : public CompositeExpression {

private:
  QString fVariable;
  QList<Object::Pointer> fArgs;

  static const QString ATT_VARIABLE;
  static const QString ATT_ARGS;

  /**
   * The seed for the hash code for all resolve expressions.
   */
  static const uint HASH_INITIAL;

public:

  ResolveExpression(const SmartPointer<IConfigurationElement>& configElement);

  ResolveExpression(Poco::XML::Element* element);

  ResolveExpression(const QString& variable, const QList<Object::Pointer>& args);

  EvaluationResult::ConstPointer Evaluate(IEvaluationContext* context) const override;

  void CollectExpressionInfo(ExpressionInfo* info) const override;

  bool operator==(const Object* object) const override;


protected:

  uint ComputeHashCode() const override;
};

} // namespace berry

#endif // __BERRY_RESOLVE_EXPRESSION_H__
