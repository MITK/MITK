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

#include "berryEqualsExpression.h"

#include "berryExpressions.h"
#include <berryIConfigurationElement.h>

#include <Poco/Hash.h>
#include <Poco/Exception.h>
#include <Poco/Bugcheck.h>

namespace berry {

const uint EqualsExpression::HASH_INITIAL= qHash("berry::EqualsExpression");


EqualsExpression::EqualsExpression(Object::Pointer expectedValue) {
  poco_assert(expectedValue.IsNotNull());

  fExpectedValue = expectedValue;
}

EqualsExpression::EqualsExpression(const IConfigurationElement::Pointer& element) {
  QString value = element->GetAttribute(ATT_VALUE);
  Expressions::CheckAttribute(ATT_VALUE, value);
  fExpectedValue = Expressions::ConvertArgument(value);
}

EqualsExpression::EqualsExpression(Poco::XML::Element* element) {
  std::string strValue = element->getAttribute(ATT_VALUE.toStdString());
  QString value(strValue.size() > 0 ? QString::fromStdString(strValue) : QString());
  Expressions::CheckAttribute(ATT_VALUE, value);
  fExpectedValue = Expressions::ConvertArgument(value);
}

EvaluationResult::ConstPointer
EqualsExpression::Evaluate(IEvaluationContext* context) const
{
  Object::ConstPointer element= context->GetDefaultVariable();
  return EvaluationResult::ValueOf(element == fExpectedValue);
}

void
EqualsExpression::CollectExpressionInfo(ExpressionInfo* info) const
{
  info->MarkDefaultVariableAccessed();
}

bool
EqualsExpression::operator==(const Object* object) const
{
  if(const EqualsExpression* that = dynamic_cast<const EqualsExpression*>(object))
  {
    return this->fExpectedValue == that->fExpectedValue;
  }
  return false;
}

uint
EqualsExpression::ComputeHashCode() const
{
  return HASH_INITIAL * HASH_FACTOR + fExpectedValue->HashCode();
}

}
