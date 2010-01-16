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

#include "berryEvaluationReference.h"

namespace berry
{

EvaluationReference::EvaluationReference(Expression::Pointer expression,
    IPropertyChangeListener::Pointer listener, const std::string& property) :
  listener(listener), property(property), postingChanges(true), resultCache(
      expression)
{
}

IPropertyChangeListener::Pointer EvaluationReference::GetListener()
{
  return listener;
}

std::string EvaluationReference::GetProperty() const
{
  return property;
}

void EvaluationReference::SetPostingChanges(bool evaluationEnabled)
{
  this->postingChanges = evaluationEnabled;
}

bool EvaluationReference::IsPostingChanges()
{
  return postingChanges;
}

void EvaluationReference::ClearResult()
{
  resultCache.ClearResult();
}

Expression::Pointer EvaluationReference::GetExpression()
{
  return resultCache.GetExpression();
}

int EvaluationReference::GetSourcePriority() const
{
  return resultCache.GetSourcePriority();
}

bool EvaluationReference::Evaluate(IEvaluationContext::Pointer context)
{
  return resultCache.Evaluate(context);
}

void EvaluationReference::SetResult(bool result)
{
  resultCache.SetResult(result);
}

}
