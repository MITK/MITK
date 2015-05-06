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

#include "berrySystemTestExpression.h"

#include "berryExpressions.h"
#include "berryExpressionPlugin.h"

#include <berryPlatform.h>
#include <berryIConfigurationElement.h>

namespace berry {

const QString SystemTestExpression::ATT_PROPERTY= "property";

const uint SystemTestExpression::HASH_INITIAL = qHash("berry::SystemTestExpression");

SystemTestExpression::SystemTestExpression(const IConfigurationElement::Pointer& element)
{
  fProperty = element->GetAttribute(ATT_PROPERTY);
  Expressions::CheckAttribute(ATT_PROPERTY, fProperty);
  fExpectedValue = element->GetAttribute(ATT_VALUE);
  Expressions::CheckAttribute(ATT_VALUE, fExpectedValue);
}

SystemTestExpression::SystemTestExpression(Poco::XML::Element* element)
{
  fProperty= QString::fromStdString(element->getAttribute(ATT_PROPERTY.toStdString()));
  Expressions::CheckAttribute(ATT_PROPERTY, fProperty.size() > 0 ? fProperty : QString());
  fExpectedValue = QString::fromStdString(element->getAttribute(ATT_VALUE.toStdString()));
  Expressions::CheckAttribute(ATT_VALUE, fExpectedValue.size() > 0 ? fExpectedValue : QString());
}

SystemTestExpression::SystemTestExpression(const QString &property, const QString &expectedValue)
 : fProperty(property), fExpectedValue(expectedValue)
{

}

EvaluationResult::ConstPointer
SystemTestExpression::Evaluate(IEvaluationContext*  /*context*/) const
{
  QString str = ExpressionPlugin::GetDefault()->GetPluginContext()->getProperty(fProperty).toString();
  if (str.isEmpty())
    return EvaluationResult::FALSE_EVAL;

  return EvaluationResult::ValueOf(str == fExpectedValue);
}

void
SystemTestExpression::CollectExpressionInfo(ExpressionInfo* info) const
{
  info->MarkSystemPropertyAccessed();
}

bool
SystemTestExpression::operator==(const Object* object) const
{
  if (const SystemTestExpression* that = dynamic_cast<const SystemTestExpression*>(object))
  {
    return this->fProperty == that->fProperty
        && this->fExpectedValue == that->fExpectedValue;
  }
  return false;
}

uint
SystemTestExpression::ComputeHashCode() const
{
  return HASH_INITIAL * HASH_FACTOR + qHash(fExpectedValue)
      * HASH_FACTOR + qHash(fProperty);
}

QString
SystemTestExpression::ToString() const
{
  return QString("<systemTest property=\"") + fProperty +
      "\" value=\"" + fExpectedValue + "\"";
  }

}
