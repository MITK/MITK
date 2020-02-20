/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryActivePartExpression.h"

#include "berryISources.h"
#include "berryIWorkbenchPart.h"

namespace berry {

const int ActivePartExpression::HASH_INITIAL = qHash(ActivePartExpression::GetStaticClassName());

ActivePartExpression::ActivePartExpression(IWorkbenchPart* activePart)
  : activePart(activePart)
{
  if (activePart == nullptr)
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
