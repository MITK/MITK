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

#include "berryActivePartExpression.h"

#include "berryISources.h"
#include "berryIWorkbenchPart.h"

namespace berry {

const int ActivePartExpression::HASH_INITIAL = qHash(ActivePartExpression::GetStaticClassName());

ActivePartExpression::ActivePartExpression(IWorkbenchPart* activePart)
  : activePart(activePart)
{
  if (activePart == NULL)
  {
    throw ctkInvalidArgumentException("The active part must not be null");
  }
}

void ActivePartExpression::CollectExpressionInfo(ExpressionInfo* info) const
{
  info->AddVariableNameAccess(ISources::ACTIVE_PART_NAME());
}

bool ActivePartExpression::operator==(const Object* object) const
{
  if (const ActivePartExpression* that = dynamic_cast<const ActivePartExpression*>(object))
  {
    return this->activePart->operator ==(that->activePart);
  }

  return false;
}

SmartPointer<const EvaluationResult> ActivePartExpression::Evaluate(IEvaluationContext* context) const
{
  const Object::ConstPointer variable = context->GetVariable(ISources::ACTIVE_PART_NAME());
  if (variable == activePart)
  {
    return EvaluationResult::TRUE_EVAL;
  }
  return EvaluationResult::FALSE_EVAL;
}

QString ActivePartExpression::ToString() const
{
  QString str = "ActivePartExpression(" + activePart->ToString() + ')';
  return str;
}

uint ActivePartExpression::ComputeHashCode() const
{
  return HASH_INITIAL * HASH_FACTOR + activePart->HashCode();
}

}
