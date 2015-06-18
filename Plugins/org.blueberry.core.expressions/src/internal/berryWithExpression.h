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
