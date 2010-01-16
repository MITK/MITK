/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "berryEvaluationResultCache.h"

#include "berrySourcePriorityNameMapping.h"

#include <osgi/framework/Exceptions.h>

namespace berry
{

EvaluationResultCache::EvaluationResultCache(Expression::Pointer expression) :
  evaluationResult(0), expression(expression), sourcePriority(
      SourcePriorityNameMapping::ComputeSourcePriority(expression))
{
}

void EvaluationResultCache::ClearResult()
{
  evaluationResult = 0;
}

bool EvaluationResultCache::Evaluate(IEvaluationContext::Pointer context)
{
  if (!expression)
  {
    return true;
  }

  if (!evaluationResult)
  {
    try
    {
      EvaluationResult result(expression->Evaluate(context));
      evaluationResult = (result == EvaluationResult::FALSE_EVAL ?
                          &EvaluationResult::FALSE_EVAL : (result == EvaluationResult::TRUE_EVAL ? &EvaluationResult::TRUE_EVAL : &EvaluationResult::NOT_LOADED));
    } catch (const CoreException& e)
    {
      /*
       * Swallow the exception. It simply means the variable is not
       * valid it some (most frequently, that the value is null). This
       * kind of information is not really useful to us, so we can
       * just treat it as null.
       */
      evaluationResult = &EvaluationResult::FALSE_EVAL;
      return false;
    }
  }

  // return true if  the result is FALSE or NOT_LOADED
  return (*evaluationResult) != EvaluationResult::FALSE_EVAL;
}

Expression::Pointer EvaluationResultCache::GetExpression()
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
    evaluationResult = &EvaluationResult::TRUE_EVAL;
  }
  else
  {
    evaluationResult = &EvaluationResult::FALSE_EVAL;
  }
}

}
