/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryWorkbenchWindowExpression.h"

#include "berryISources.h"
#include "berryIWorkbenchWindow.h"

namespace berry {

const int WorkbenchWindowExpression::HASH_INITIAL = qHash(WorkbenchWindowExpression::GetStaticClassName());

WorkbenchWindowExpression::WorkbenchWindowExpression(IWorkbenchWindow* window)
  : window(window)
{
}

void WorkbenchWindowExpression::CollectExpressionInfo(ExpressionInfo* info) const
{
  if (window != nullptr)
  {
    info->AddVariableNameAccess(ISources::ACTIVE_WORKBENCH_WINDOW_NAME());
  }
}

bool WorkbenchWindowExpression::operator==(const Object* object) const
{
  if (const WorkbenchWindowExpression* that = dynamic_cast<const WorkbenchWindowExpression*>(object))
  {
   return this->window->operator ==(that->window);
  }

  return false;
}

SmartPointer<const EvaluationResult> WorkbenchWindowExpression::Evaluate(IEvaluationContext* context) const
{
  if (window != nullptr)
  {
    Object::ConstPointer value = context->GetVariable(ISources::ACTIVE_WORKBENCH_WINDOW_NAME());
    if (value == window)
    {
      return EvaluationResult::TRUE_EVAL;
    }
  }

  return EvaluationResult::FALSE_EVAL;
}

QString WorkbenchWindowExpression::ToString() const
{
  QString str = "WorkbenchWindowExpression(" + window->ToString() + ')';
  return str;
}

uint WorkbenchWindowExpression::ComputeHashCode() const
{
  return HASH_INITIAL * HASH_FACTOR + window->HashCode();
}

IWorkbenchWindow* WorkbenchWindowExpression::GetWindow() const
{
  return window;
}

}
