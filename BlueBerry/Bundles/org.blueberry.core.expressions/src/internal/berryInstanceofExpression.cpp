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

#include "berryInstanceofExpression.h"

#include "berryExpressions.h"
#include <berryIConfigurationElement.h>

namespace berry {

const uint InstanceofExpression::HASH_INITIAL= qHash("berry::InstanceofExpression");

InstanceofExpression::InstanceofExpression(const IConfigurationElement::Pointer& element)
{
  fTypeName = element->GetAttribute(ATT_VALUE);
  Expressions::CheckAttribute(ATT_VALUE, fTypeName);
}

InstanceofExpression::InstanceofExpression(Poco::XML::Element* element)
{
  std::string value = element->getAttribute(ATT_VALUE.toStdString());
  fTypeName = value.size() > 0 ? QString::fromStdString(value) : QString();
  Expressions::CheckAttribute(ATT_VALUE, fTypeName);
}

InstanceofExpression::InstanceofExpression(const QString& typeName)
 : fTypeName(typeName)
{

}

EvaluationResult::ConstPointer
InstanceofExpression::Evaluate(IEvaluationContext* context) const
{
  Object::ConstPointer element= context->GetDefaultVariable();
  return EvaluationResult::ValueOf(Expressions::IsInstanceOf(element.GetPointer(), fTypeName));
}

void
InstanceofExpression::CollectExpressionInfo(ExpressionInfo* info) const
{
  info->MarkDefaultVariableAccessed();
}

bool
InstanceofExpression::operator==(const Object* object) const
{
  if (const InstanceofExpression* that = dynamic_cast<const InstanceofExpression*>(object))
  {
    return this->fTypeName == that->fTypeName;
  }
  return false;
}

QString
InstanceofExpression::ToString() const
{
  return QString("<instanceof value=\"") + fTypeName + "\"/>";
}

uint
InstanceofExpression::ComputeHashCode() const
{
  return HASH_INITIAL * HASH_FACTOR + qHash(fTypeName);
}

}
