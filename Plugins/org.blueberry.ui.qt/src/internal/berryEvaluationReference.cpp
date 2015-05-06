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


#include "berryEvaluationReference.h"

#include <berryIPropertyChangeListener.h>

namespace berry {

EvaluationReference::EvaluationReference(const SmartPointer<Expression>& expression,
                                         IPropertyChangeListener* listener,
                                         const QString& property)
  : EvaluationResultCache(expression),
    listener(listener), property(property), postingChanges(true)
{
}

IPropertyChangeListener* EvaluationReference::GetListener() const
{
  return listener;
}

QString EvaluationReference::GetProperty() const
{
  return property;
}

void EvaluationReference::SetPostingChanges(bool evaluationEnabled)
{
  this->postingChanges = evaluationEnabled;
}

bool EvaluationReference::IsPostingChanges() const
{
  return postingChanges;
}

}
