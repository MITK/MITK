/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "berryEvaluationResultCache.h"

#include "berrySourcePriorityNameMapping.h"
#include "berryEvaluationResult.h"

#include <berryCoreException.h>

namespace berry {

EvaluationResultCache::EvaluationResultCache(const SmartPointer<Expression>& expression)
  : expression(expression),
    sourcePriority(SourcePriorityNameMapping::ComputeSourcePriority(expression))
{
}

EvaluationResultCache::~EvaluationResultCache()
{
}

void EvaluationResultCache::ClearResult()
{
  evaluationResult = nullptr;
}

bool EvaluationResultCache::Evaluate(IEvaluationContext* context) const
{
  if (expression.IsNull())
  {
    return true;
  }

  if (evaluationResult.IsNull())
  {
    try
    {
      evaluationResult = expression->Evaluate(context);
    }
    catch (const CoreException& /*e*/)
    {
      /*
       * Swallow the exception. It simply means the variable is not
       * valid it some (most frequently, that the value is null). This
       * kind of information is not really useful to us, so we can
       * just treat it as null.
       */
      evaluationResult = EvaluationResult::FALSE_EVAL;
      return false;
    }
  }

  // return true if  the result is FALSE or NOT_LOADED
  return evaluationResult != EvaluationResult::FALSE_EVAL;
}

SmartPointer<Expression> EvaluationResultCache::GetExpression() const
{
  return expression;
}

int EvaluationResultCache::GetSourcePriority() const
{
  return sourcePriority;
}

void EvaluationResultCache::SetResult(bool result)
{
  if (result)
  {
    evaluationResult = EvaluationResult::TRUE_EVAL;
  }
  else
  {
    evaluationResult = EvaluationResult::FALSE_EVAL;
  }
}

}
