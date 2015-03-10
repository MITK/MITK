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
  static const std::size_t HASH_INITIAL;

  QString fDefinitionId;


public:

  ReferenceExpression(const QString& definitionId);

  ReferenceExpression(SmartPointer<IConfigurationElement> element);

  ReferenceExpression(Poco::XML::Element* element);

  EvaluationResult::ConstPointer Evaluate(IEvaluationContext* context) const;

  void CollectExpressionInfo(ExpressionInfo* info) const;

  bool operator==(const Object* object) const;


protected:
  uint ComputeHashCode() const;
};

} // namespace berry

#endif // __BERRY_REFERENCE_EXPRESSION_H__
