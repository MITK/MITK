/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryResolveExpression.h"

#include "berryExpressions.h"
#include "berryEvaluationContext.h"
#include "berryExpressionStatus.h"

#include <berryIConfigurationElement.h>
#include <berryCoreException.h>

namespace berry {

const QString ResolveExpression::ATT_VARIABLE= "variable";
const QString ResolveExpression::ATT_ARGS= "args";
const uint ResolveExpression::HASH_INITIAL = qHash("berry::ResolveExpression");

ResolveExpression::ResolveExpression(const IConfigurationElement::Pointer& configElement)
{
  fVariable = configElement->GetAttribute(ATT_VARIABLE);
  Expressions::CheckAttribute(ATT_VARIABLE, fVariable);
  fArgs = Expressions::GetArguments(configElement, ATT_ARGS);
}

ResolveExpression::ResolveExpression(Poco::XML::Element* element)
{
  fVariable = QString::fromStdString(element->getAttribute(ATT_VARIABLE.toStdString()));
  Expressions::CheckAttribute(ATT_VARIABLE, fVariable.size() > 0 ? fVariable : QString());
  fArgs = Expressions::GetArguments(element, ATT_ARGS);
}

ResolveExpression::ResolveExpression(const QString& variable, const QList<Object::Pointer>& args)
 : fVariable(variable), fArgs(args)
{

}

EvaluationResult::ConstPointer
ResolveExpression::Evaluate(IEvaluationContext* context) const
{
  Object::ConstPointer variable= context->ResolveVariable(fVariable, fArgs);
  if (variable.IsNull())
  {
    IStatus::Pointer status(new ExpressionStatus(ExpressionStatus::VARIABLE_NOT_DEFINED,
                                                 QString("The variable %1 is not defined.").arg(fVariable),
                                                 BERRY_STATUS_LOC));
    throw CoreException(status);
  }
  EvaluationContext evalContext(context, variable);
  return this->EvaluateAnd(&evalContext);
}

void
ResolveExpression::CollectExpressionInfo(ExpressionInfo* info) const
{
  ExpressionInfo other;
  this->CompositeExpression::CollectExpressionInfo(&other);
  if (other.HasDefaultVariableAccess())
  {
    info->AddVariableNameAccess(fVariable);
  }
  info->MergeExceptDefaultVariable(&other);
}

bool
ResolveExpression::operator==(const Object* object) const
{
  if (const ResolveExpression* that = dynamic_cast<const ResolveExpression*>(object))
  {
    return this->fVariable == that->fVariable
        && this->Equals(this->fArgs, that->fArgs)
        && this->Equals(this->fExpressions, that->fExpressions);
  }
  return false;
}

uint
ResolveExpression::ComputeHashCode() const
{
  return HASH_INITIAL * HASH_FACTOR + this->HashCode(fExpressions)
      * HASH_FACTOR + this->HashCode(fArgs)
      * HASH_FACTOR + qHash(fVariable);
}

} // namespace berry
