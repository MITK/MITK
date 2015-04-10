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

#include "berryReferenceExpression.h"

#include "berryExpressions.h"

#include <berryIConfigurationElement.h>
#include <berryCoreException.h>

namespace berry {

const QString ReferenceExpression::ATT_DEFINITION_ID = "definitionId";
const uint ReferenceExpression::HASH_INITIAL= qHash(QString("berry::ReferenceExpression"));

DefinitionRegistry ReferenceExpression::fgDefinitionRegistry = DefinitionRegistry();

DefinitionRegistry&
ReferenceExpression::GetDefinitionRegistry()
{
  return fgDefinitionRegistry;
}

ReferenceExpression::ReferenceExpression(const QString &definitionId)
{
  fDefinitionId= definitionId;
}

ReferenceExpression::ReferenceExpression(IConfigurationElement::Pointer element)
{
  fDefinitionId = element->GetAttribute(ATT_DEFINITION_ID);
  Expressions::CheckAttribute(ATT_DEFINITION_ID, fDefinitionId);
}

ReferenceExpression::ReferenceExpression(Poco::XML::Element* element)
{
  fDefinitionId = QString::fromStdString(element->getAttribute(ATT_DEFINITION_ID.toStdString()));
  Expressions::CheckAttribute(ATT_DEFINITION_ID, fDefinitionId.size() > 0 ? fDefinitionId : QString());
}

EvaluationResult::ConstPointer
ReferenceExpression::Evaluate(IEvaluationContext* context) const
{
  Expression::Pointer expr= GetDefinitionRegistry().GetExpression(fDefinitionId);
  return expr->Evaluate(context);
}

void
ReferenceExpression::CollectExpressionInfo(ExpressionInfo* info) const
{
  Expression::Pointer expr;
  try
  {
    expr= GetDefinitionRegistry().GetExpression(fDefinitionId);
  }
  catch (const CoreException& /*e*/)
  {
    // We didn't find the expression definition. So no
    // expression info can be collected.
    return;
  }
  expr->CollectExpressionInfo(info);
}

bool
ReferenceExpression::operator==(const Object* object) const
{
  if (const ReferenceExpression* that = dynamic_cast<const ReferenceExpression*>(object))
  {
    return this->fDefinitionId == that->fDefinitionId;
  }
  return false;
}

uint
ReferenceExpression::ComputeHashCode() const
{
  return HASH_INITIAL * HASH_FACTOR + qHash(fDefinitionId);
}

}
