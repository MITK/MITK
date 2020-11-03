/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
