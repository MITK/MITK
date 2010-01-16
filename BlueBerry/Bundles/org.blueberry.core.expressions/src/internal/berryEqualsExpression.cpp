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

#include "berryEqualsExpression.h"

#include "berryExpressions.h"

#include <Poco/Hash.h>
#include <Poco/Exception.h>
#include <Poco/Bugcheck.h>

namespace berry {

const std::size_t EqualsExpression::HASH_INITIAL= Poco::Hash<std::string>()("berry::EqualsExpression");


EqualsExpression::EqualsExpression(Object::Pointer expectedValue) {
  poco_assert(expectedValue.IsNotNull());

  fExpectedValue = expectedValue;
}

EqualsExpression::EqualsExpression(IConfigurationElement::Pointer element) {
  std::string value;
  bool result = element->GetAttribute(ATT_VALUE, value);
  Expressions::CheckAttribute(ATT_VALUE, result);
  fExpectedValue = Expressions::ConvertArgument(value, result);
}

EqualsExpression::EqualsExpression(Poco::XML::Element* element) {
  std::string value = element->getAttribute(ATT_VALUE);
  Expressions::CheckAttribute(ATT_VALUE, value.size() > 0);
  fExpectedValue = Expressions::ConvertArgument(value);
}

EvaluationResult
EqualsExpression::Evaluate(IEvaluationContext* context) {
  Object::Pointer element= context->GetDefaultVariable();
  return EvaluationResult::ValueOf(element == fExpectedValue);
}

void
EqualsExpression::CollectExpressionInfo(ExpressionInfo* info) {
  info->MarkDefaultVariableAccessed();
}

bool
EqualsExpression::operator==(Expression& object) {

  try {
    EqualsExpression& that = dynamic_cast<EqualsExpression&>(object);
    return this->fExpectedValue == that.fExpectedValue;
  }
  catch (std::bad_cast)
  {
    return false;
  }
}

std::size_t
EqualsExpression::ComputeHashCode() {
  return HASH_INITIAL * HASH_FACTOR + fExpectedValue->HashCode();
}

}
