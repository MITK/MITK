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
  if (window != NULL)
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
  if (window != NULL)
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
