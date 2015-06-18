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

#ifndef __BERRY_INSTANCEOF_EXPRESSION_H__
#define __BERRY_INSTANCEOF_EXPRESSION_H__

#include "berryExpression.h"

#include "Poco/DOM/Element.h"

namespace berry {

struct IConfigurationElement;

struct InstanceofExpression : public Expression {

private:

  /**
    * The seed for the hash code for all instance of expressions.
    */
  static const uint HASH_INITIAL;

  QString fTypeName;


public:

  InstanceofExpression(const SmartPointer<IConfigurationElement>& element);

  InstanceofExpression(Poco::XML::Element* element);

  InstanceofExpression(const QString &typeName);

  /*
   * @see org.blueberry.jdt.internal.corext.refactoring.participants.Expression#evaluate(java.lang.Object)
   */
  EvaluationResult::ConstPointer Evaluate(IEvaluationContext* context) const override;

  void CollectExpressionInfo(ExpressionInfo* info) const override;

  bool operator==(const Object* object) const override;

  //---- Debugging ---------------------------------------------------

    /*
     * @see java.lang.Object#toString()
     */
  QString ToString() const override;

protected:

  uint ComputeHashCode() const override;


};

} // namespace berry

#endif // __BERRY_INSTANCEOF_EXPRESSION_H__
