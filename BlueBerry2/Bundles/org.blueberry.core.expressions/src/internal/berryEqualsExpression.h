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

#ifndef __BERRY_EQUALS_EXPRESSION_H__
#define __BERRY_EQUALS_EXPRESSION_H__

#include "../berryExpression.h"
#include "../berryExpressionInfo.h"
#include "../berryIEvaluationContext.h"

#include "service/berryIConfigurationElement.h"

#include "Poco/DOM/Element.h"

namespace berry {

class EqualsExpression : public Expression {
  /**
   * The seed for the hash code for all equals expressions.
   */
private:

  static const std::size_t HASH_INITIAL;

  Object::Pointer fExpectedValue;

public:

  EqualsExpression(const Object::Pointer expectedValue);

  EqualsExpression(SmartPointer<IConfigurationElement> element);

  EqualsExpression(Poco::XML::Element* element);

  EvaluationResult Evaluate(IEvaluationContext::Pointer context);

  void CollectExpressionInfo(ExpressionInfo* info);

  bool operator==(Expression& object);

protected:

  std::size_t ComputeHashCode();
};

} // namespace berry

#endif // __BERRY_EQUALS_EXPRESSION_H__
