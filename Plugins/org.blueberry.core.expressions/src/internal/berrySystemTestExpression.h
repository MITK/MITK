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

#ifndef __BERRY_SYSTEM_TEXT_EXPRESSION_H__
#define __BERRY_SYSTEM_TEXT_EXPRESSION_H__

#include "berryExpression.h"

#include "Poco/DOM/Element.h"

namespace berry {

struct IConfigurationElement;

class SystemTestExpression : public Expression {

private:
  QString fProperty;
  QString fExpectedValue;

  static const QString ATT_PROPERTY;

  /**
   * The seed for the hash code for all system test expressions.
   */
  static const uint HASH_INITIAL;

public:

  SystemTestExpression(const SmartPointer<IConfigurationElement>& element);

  SystemTestExpression(Poco::XML::Element* element);

  SystemTestExpression(const QString& property, const QString& expectedValue);

  EvaluationResult::ConstPointer Evaluate(IEvaluationContext* context) const override;

  void CollectExpressionInfo(ExpressionInfo* info) const override;

  bool operator==(const Object* object) const override;

  QString ToString() const override;

protected:

  uint ComputeHashCode() const override ;

};

} // namespace berry

#endif // __BERRY_SYSTEM_TEXT_EXPRESSION_H__
