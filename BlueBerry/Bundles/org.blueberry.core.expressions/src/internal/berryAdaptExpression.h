/*=========================================================================

Program:   BlueBerry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef BERRYADAPTEXPRESSION_H_
#define BERRYADAPTEXPRESSION_H_

#include "berryCompositeExpression.h"

#include "../berryEvaluationContext.h"
#include "../berryEvaluationResult.h"
#include "../berryExpressionInfo.h"

#include "service/berryIConfigurationElement.h"

#include "Poco/DOM/Node.h"

#include <string>

namespace berry {

class AdaptExpression : public CompositeExpression
{

private:

  static const std::string ATT_TYPE;

  /**
   * The seed for the hash code for all adapt expressions.
   */
  static const std::size_t HASH_INITIAL;

  std::string fTypeName;


public:

  AdaptExpression(SmartPointer<IConfigurationElement> configElement);

  AdaptExpression(Poco::XML::Node* element);

  AdaptExpression(const std::string& typeName);

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
  EvaluationResult Evaluate(IEvaluationContext* context);

  void CollectExpressionInfo(ExpressionInfo* info);


  protected:
    std::size_t ComputeHashCode();
};

}  // namespace berry

#endif /*BERRYADAPTEXPRESSION_H_*/
