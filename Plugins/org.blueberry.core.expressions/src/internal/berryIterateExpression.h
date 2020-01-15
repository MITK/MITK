/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __BERRY_ITERATE_EXPRESSION_H__
#define __BERRY_ITERATE_EXPRESSION_H__

#include "berryCompositeExpression.h"

#include "Poco/DOM/Element.h"
#include "Poco/Any.h"


namespace berry {

struct IConfigurationElement;

struct IterateExpression : public CompositeExpression {

private:

  static const QString ATT_OPERATOR;
  static const QString ATT_IF_EMPTY;
  static const int OR;
  static const int AND;

  /**
   * The seed for the hash code for all iterate expressions.
   */
  static const uint HASH_INITIAL;

  int fOperator;
  int fEmptyResult;

public:

  IterateExpression(const SmartPointer<IConfigurationElement>& configElement);

  IterateExpression(Poco::XML::Element* element);

  IterateExpression(const QString& opValue);

  IterateExpression(const QString& opValue, const QString& ifEmpty);

private:

  void InitializeOperatorValue(const QString& opValue);

  void InitializeEmptyResultValue(const QString& value);


public:

  /* (non-Javadoc)
    * @see Expression#evaluate(IVariablePool)
    */
  EvaluationResult::ConstPointer Evaluate(IEvaluationContext* context) const override;

  void CollectExpressionInfo(ExpressionInfo* info) const override;

  bool operator==(const Object* object) const override;


protected:

  uint ComputeHashCode() const override;
};

} // namespace berry

#endif // __BERRY_ITERATE_EXPRESSION_H__
