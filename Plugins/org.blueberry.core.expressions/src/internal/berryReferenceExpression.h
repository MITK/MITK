/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __BERRY_REFERENCE_EXPRESSION_H__
#define __BERRY_REFERENCE_EXPRESSION_H__

#include "berryDefinitionRegistry.h"

#include "Poco/DOM/Element.h"

namespace berry {

/**
 * This class makes use of the <b>org.blueberry.core.expressions.definitions</b>
 * extension point to evaluate the current context against pre-defined
 * expressions. It provides core expression re-use.
 */
class ReferenceExpression : public Expression {

private:

  // consider making this a more general extension manager
  // for now it's just part of the reference expression
  static DefinitionRegistry fgDefinitionRegistry;

  static DefinitionRegistry& GetDefinitionRegistry();

  static const QString ATT_DEFINITION_ID;

  /**
   * The seed for the hash code for all equals expressions.
   */
  static const uint HASH_INITIAL;

  QString fDefinitionId;


public:

  ReferenceExpression(const QString& definitionId);

  ReferenceExpression(SmartPointer<IConfigurationElement> element);

  ReferenceExpression(Poco::XML::Element* element);

  EvaluationResult::ConstPointer Evaluate(IEvaluationContext* context) const override;

  void CollectExpressionInfo(ExpressionInfo* info) const override;

  bool operator==(const Object* object) const override;


protected:
  uint ComputeHashCode() const override;
};

} // namespace berry

#endif // __BERRY_REFERENCE_EXPRESSION_H__
