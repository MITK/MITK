/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "berryEvaluationService.h"

#include "berryEvaluationReference.h"
#include "berryEvaluationAuthority.h"
#include "berryExpression.h"

#include <berryIEvaluationContext.h>

#include <QStringList>

namespace berry {

EvaluationService::EvaluationService()
  : evaluationAuthority(new EvaluationAuthority())
{
  evaluationAuthority->Register();
}

EvaluationService::~EvaluationService()
{
  this->Dispose();
}

IEvaluationReference::Pointer EvaluationService::AddEvaluationListener(const SmartPointer<Expression>& expression,
                                                                       IPropertyChangeListener* listener,
                                                                       const QString& property)
{
  IEvaluationReference::Pointer expressionReference(new EvaluationReference(
                                                      expression, listener, property));

  evaluationAuthority->AddEvaluationListener(expressionReference);
  return expressionReference;
}

void EvaluationService::AddEvaluationReference(const SmartPointer<IEvaluationReference>& ref)
{
  QString msg("Invalid type: ");
  msg.append(ref->GetClassName());
  Q_ASSERT_X(ref.Cast<EvaluationReference>(), "AddEvaluationReference", qPrintable(msg));
  evaluationAuthority->AddEvaluationListener(ref);
}

void EvaluationService::RemoveEvaluationListener(const SmartPointer<IEvaluationReference>& ref)
{
  evaluationAuthority->RemoveEvaluationListener(ref);
}

void EvaluationService::AddSourceProvider(const SmartPointer<ISourceProvider>& provider)
{
  evaluationAuthority->AddSourceProvider(provider);
}

void EvaluationService::RemoveSourceProvider(const SmartPointer<ISourceProvider>& provider)
{
  evaluationAuthority->RemoveSourceProvider(provider);
}

void EvaluationService::Dispose()
{
  evaluationAuthority->Dispose();
}

SmartPointer<IEvaluationContext> EvaluationService::GetCurrentState() const
{
  return evaluationAuthority->GetCurrentState();
}

void EvaluationService::AddServiceListener(IPropertyChangeListener* listener)
{
  evaluationAuthority->AddServiceListener(listener);
}

void EvaluationService::RemoveServiceListener(IPropertyChangeListener* listener)
{
  evaluationAuthority->RemoveServiceListener(listener);
}

void EvaluationService::RequestEvaluation(const QString& propertyName)
{
  QStringList propertyNames;
  propertyNames.push_back(propertyName);
  evaluationAuthority->SourceChanged(propertyNames);
}

//void EvaluationService::UpdateShellKludge()
//{
//  evaluationAuthority->UpdateShellKludge();
//}

}
