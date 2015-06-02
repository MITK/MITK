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
