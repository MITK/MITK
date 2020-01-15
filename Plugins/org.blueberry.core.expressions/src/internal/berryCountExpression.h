/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __BERRY_COUNT_EXPRESSION_H__
#define __BERRY_COUNT_EXPRESSION_H__

#include "berryExpression.h"
#include "berryIEvaluationContext.h"
#include "berryExpressionInfo.h"

#include "Poco/DOM/Element.h"

#include <string>

namespace berry {

struct IConfigurationElement;

class CountExpression : public Expression {

private:
  static const int ANY_NUMBER;
  static const int EXACT;
   static const int ONE_OR_MORE;
   static const int NONE_OR_ONE;
   static const int NONE;
   static const int UNKNOWN;

  /**
   * The seed for the hash code for all count expressions.
   */
  static const uint HASH_INITIAL;

  int fMode;
  int fSize;

  void InitializeSize(QString size);

public:

  CountExpression(const SmartPointer<IConfigurationElement>& configElement);

  CountExpression(Poco::XML::Element* element);

  CountExpression(const QString& size);

  EvaluationResult::ConstPointer Evaluate(IEvaluationContext* context) const override;

  void CollectExpressionInfo(ExpressionInfo* info) const override;

  bool operator==(const Object* object) const override;

protected:

  uint ComputeHashCode() const override;

};

} // namespace berry

#endif // __BERRY_COUNT_EXPRESSION_H__
