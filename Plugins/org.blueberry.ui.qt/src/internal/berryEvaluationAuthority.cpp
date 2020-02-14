/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "berryEvaluationAuthority.h"

#include "berryCommandTracing.h"
#include "berryExpressionInfo.h"
#include "berryExpression.h"
#include "berryEvaluationReference.h"
#include "berryPolicy.h"
#include "berryWorkbenchPlugin.h"

#include <berryObjects.h>
#include <berryISources.h>
#include <berryIEvaluationService.h>

#include <QStringList>
#include <QString>

namespace berry {

const QString EvaluationAuthority::COMPONENT = "EVALUATION";


QStringList EvaluationAuthority::GetNames(const SmartPointer<IEvaluationReference>& ref) const
{
  ExpressionInfo info;
  ref->GetExpression()->CollectExpressionInfo(&info);
  QSet<QString> allNames = info.GetAccessedVariableNames();
  if (info.HasDefaultVariableAccess())
  {
    allNames << ISources::ACTIVE_CURRENT_SELECTION_NAME();
  }

  allNames.unite(info.GetAccessedPropertyNames());
  return allNames.toList();
}

void EvaluationAuthority::RefsWithSameExpression(const QList<SmartPointer<EvaluationReference> >& refs)
{
  int k = 0;
  while (k < refs.size() && !refs[k]->IsPostingChanges())
  {
    k++;
  }
  if (k >= refs.size())
  {
    return;
  }
  EvaluationReference::Pointer ref = refs[k];
  bool oldValue = Evaluate(ref);
  ref->ClearResult();
  const bool newValue = Evaluate(ref);
  if (oldValue != newValue)
  {
    FirePropertyChange(ref, ValueOf(oldValue), ValueOf(newValue));
  }
  for (k++; k < refs.size(); k++)
  {
    ref = refs[k];
    // this is not as expensive as it looks
    if (ref->IsPostingChanges())
    {
      oldValue = Evaluate(ref);
      if (oldValue != newValue)
      {
        ref->SetResult(newValue);
        FirePropertyChange(ref, ValueOf(oldValue),
                           ValueOf(newValue));
      }
    }
  }
}

void EvaluationAuthority::StartSourceChange(const QStringList& sourceNames)
{
  if (Policy::DEBUG_SOURCES())
  {
    CommandTracing::PrintTrace(COMPONENT, "start source changed: " + sourceNames.join(", "));
  }
  notifying++;
  if (notifying == 1)
  {
    FireServiceChange(IEvaluationService::PROP_NOTIFYING, ValueOf(false),
                      ValueOf(true));
  }
}

void EvaluationAuthority::EndSourceChange(const QStringList& sourceNames)
{
  if (Policy::DEBUG_SOURCES())
  {
    CommandTracing::PrintTrace(COMPONENT, "end source changed: " + sourceNames.join(", "));
  }
  if (notifying == 1)
  {
    FireServiceChange(IEvaluationService::PROP_NOTIFYING, ValueOf(true),
                      ValueOf(false));
  }
  notifying--;
}

void EvaluationAuthority::FirePropertyChange(const SmartPointer<IEvaluationReference>& ref,
                                             Object::Pointer oldValue, Object::Pointer newValue)
{
  PropertyChangeEvent::Pointer event(new PropertyChangeEvent(ref, ref->GetProperty(), oldValue,
                                                             newValue));
  ref->GetListener()->PropertyChange(event);
}

void EvaluationAuthority::FireServiceChange(const QString& property, Object::Pointer oldValue,
                                            Object::Pointer newValue)
{
  PropertyChangeEvent::Pointer event(
        new PropertyChangeEvent(Object::Pointer(this), property, oldValue, newValue));
  serviceListeners.propertyChange(event);
}

void EvaluationAuthority::ServiceChangeException(const std::exception &exc)
{
  WorkbenchPlugin::Log(exc.what());
}

Object::Pointer EvaluationAuthority::ValueOf(bool result)
{
  return ObjectBool::Pointer(new ObjectBool(result));
}

void EvaluationAuthority::SourceChanged(int /*sourcePriority*/)
{
  // no-op, we want the other one
}

void EvaluationAuthority::SourceChanged(const QStringList& sourceNames)
{
  struct SourceChangeScope {
    EvaluationAuthority* const ea;
    const QStringList& sourceNames;
    SourceChangeScope(EvaluationAuthority* ea, const QStringList& sourceNames)
      : ea(ea), sourceNames(sourceNames)
    {
      ea->StartSourceChange(sourceNames);
    }
    ~SourceChangeScope()
    {
      ea->EndSourceChange(sourceNames);
    }
  };

  SourceChangeScope(this, sourceNames);
  // evaluations to recompute
  for (int i = 0; i < sourceNames.size(); i++)
  {
    if (cachesBySourceName.contains(sourceNames[i]))
    {
      const ExprToEvalsMapType& cachesByExpression = cachesBySourceName[sourceNames[i]];
      QList<QSet<EvaluationReference::Pointer> > expressionCaches = cachesByExpression.values();
      for (int j = 0; j < expressionCaches.size(); j++)
      {
        if (!(expressionCaches[j].isEmpty()))
        {
          QList<EvaluationReference::Pointer> refs = expressionCaches[j].toList();
          RefsWithSameExpression(refs);
        }
      }
    }
  }
}

EvaluationAuthority::EvaluationAuthority()
  : serviceExceptionHandler(this, &EvaluationAuthority::ServiceChangeException), notifying(0)
{
  serviceListeners.propertyChange.SetExceptionHandler(serviceExceptionHandler);
}

SmartPointer<const Shell> EvaluationAuthority::GetActiveShell() const
{
  return GetVariable(ISources::ACTIVE_SHELL_NAME()).Cast<const Shell>();
}

void EvaluationAuthority::AddEvaluationListener(const SmartPointer<IEvaluationReference>& ref)
{
  // we update the source priority bucket sort of activations.
  QStringList sourceNames = GetNames(ref);
  for (int i = 0; i < sourceNames.size(); i++)
  {
    ExprToEvalsMapType& cachesByExpression = cachesBySourceName[sourceNames[i]];
    const Expression::Pointer expression = ref->GetExpression();
    cachesByExpression[expression].insert(ref.Cast<EvaluationReference>());
  }

  bool result = Evaluate(ref);
  FirePropertyChange(ref, Object::Pointer(nullptr), ValueOf(result));
}

void EvaluationAuthority::RemoveEvaluationListener(const SmartPointer<IEvaluationReference>& ref)
{
  // Next we update the source priority bucket sort of activations.
  QStringList sourceNames = GetNames(ref);
  for (int i = 0; i < sourceNames.size(); i++)
  {
    if (cachesBySourceName.contains(sourceNames[i]))
    {
      ExprToEvalsMapType& cachesByExpression = cachesBySourceName[sourceNames[i]];
      if (cachesByExpression.contains(ref->GetExpression()))
      {
        QSet<EvaluationReference::Pointer>& caches = cachesByExpression[ref->GetExpression()];
        caches.remove(ref.Cast<EvaluationReference>());
        if (caches.isEmpty())
        {
          cachesByExpression.remove(ref->GetExpression());
        }
      }
      if (cachesByExpression.isEmpty())
      {
        cachesBySourceName.remove(sourceNames[i]);
      }
    }
  }
  bool result = Evaluate(ref);
  FirePropertyChange(ref, ValueOf(result), Object::Pointer(nullptr));
}

void EvaluationAuthority::AddServiceListener(IPropertyChangeListener* listener)
{
  serviceListeners.AddListener(listener);
}

void EvaluationAuthority::RemoveServiceListener(IPropertyChangeListener* listener)
{
  serviceListeners.RemoveListener(listener);
}

}
