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

  EvaluationResult::ConstPointer Evaluate(IEvaluationContext* context) const;

  void CollectExpressionInfo(ExpressionInfo* info) const;

  bool operator==(const Object* object) const;

protected:

  uint ComputeHashCode() const;

};

} // namespace berry

#endif // __BERRY_COUNT_EXPRESSION_H__
