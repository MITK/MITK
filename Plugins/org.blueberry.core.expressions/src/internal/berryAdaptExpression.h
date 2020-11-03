/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYADAPTEXPRESSION_H_
#define BERRYADAPTEXPRESSION_H_

#include "berryCompositeExpression.h"

namespace Poco {
namespace XML {
class Node;
}
}

namespace berry {

struct IConfigurationElement;

class AdaptExpression : public CompositeExpression
{

private:

  static const QString ATT_TYPE;

  /**
   * The seed for the hash code for all adapt expressions.
   */
  static const uint HASH_INITIAL;

  QString fTypeName;


public:

  AdaptExpression(SmartPointer<IConfigurationElement> configElement);

  AdaptExpression(Poco::XML::Node* element);

  AdaptExpression(const QString& typeName);

//  bool equals(final Object object) {
//    if (!(object instanceof AdaptExpression))
//      return false;
//
//    final AdaptExpression that= (AdaptExpression)object;
//    return this.fTypeName.equals(that.fTypeName)
//        && equals(this.fExpressions, that.fExpressions);
//  }


  /* (non-Javadoc)
   * @see Expression#evaluate(IVariablePool)
   */
  EvaluationResult::ConstPointer Evaluate(IEvaluationContext* context) const override;

  void CollectExpressionInfo(ExpressionInfo* info) const override;


protected:

  uint ComputeHashCode() const override;
};

}  // namespace berry

#endif /*BERRYADAPTEXPRESSION_H_*/
