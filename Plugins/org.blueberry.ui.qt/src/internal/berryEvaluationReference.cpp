/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
