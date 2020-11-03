/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryWithExpression.h"

#include "berryExpressions.h"
#include "berryEvaluationContext.h"
#include "berryExpressionStatus.h"

#include <berryIConfigurationElement.h>
#include <berryCoreException.h>

namespace berry {

const QString WithExpression::ATT_VARIABLE= "variable";

const uint WithExpression::HASH_INITIAL= qHash("berry::WithExpression");

WithExpression::WithExpression(const IConfigurationElement::Pointer& configElement)
{
  fVariable = configElement->GetAttribute(ATT_VARIABLE);
  Expressions::CheckAttribute(ATT_VARIABLE, fVariable);
}

WithExpression::WithExpression(Poco::XML::Element* element)
{
  fVariable = QString::fromStdString(element->getAttribute(ATT_VARIABLE.toStdString()));
  Expressions::CheckAttribute(ATT_VARIABLE, fVariable.length() > 0 ? fVariable : QString());
}

WithExpression::WithExpression(const QString& variable)
 : fVariable(variable)
{

}

bool
WithExpression::operator==(const Object* object) const
{
  if (const WithExpression* that = dynamic_cast<const WithExpression*>(object))
  {
    return this->fVariable == that->fVariable &&
            this->Equals(this->fExpressions, that->fExpressions);
  }
  return false;
}

EvaluationResult::ConstPointer
WithExpression::Evaluate(IEvaluationContext* context) const
{
  Object::ConstPointer variable(context->GetVariable(fVariable));
  if (variable.IsNull())
  {
    IStatus::Pointer status(new ExpressionStatus(ExpressionStatus::VARIABLE_NOT_DEFINED,
                                                 QString("The variable %1 is not defined").arg(fVariable),
                                                 BERRY_STATUS_LOC));
    throw CoreException(status);
  }
  if (variable == IEvaluationContext::UNDEFINED_VARIABLE)
  {
    return EvaluationResult::FALSE_EVAL;
  }
  EvaluationContext::Pointer scope(new EvaluationContext(context, variable));
  return this->EvaluateAnd(scope.GetPointer());
}

void
WithExpression::CollectExpressionInfo(ExpressionInfo* info) const
{
  auto  other = new ExpressionInfo();
  CompositeExpression::CollectExpressionInfo(other);
  if (other->HasDefaultVariableAccess())
  {
    info->AddVariableNameAccess(fVariable);
  }
  info->MergeExceptDefaultVariable(other);
}

uint
WithExpression::ComputeHashCode() const
{
  return HASH_INITIAL * HASH_FACTOR + this->HashCode(fExpressions)
  * HASH_FACTOR + qHash(fVariable);
}

}
