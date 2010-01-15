/*=========================================================================

 Program:   openCherry Platform
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

#include "cherryEvaluationAuthority.h"
#include "../cherryISources.h"
#include "cherryWorkbenchPlugin.h"
#include "../services/cherryIEvaluationService.h"

#include <cherrySafeRunner.h>

namespace cherry
{

const std::string EvaluationAuthority::COMPONENT = "EVALUATION";

EvaluationAuthority::SafeServiceChangeRunnable::SafeServiceChangeRunnable(
    EvaluationAuthority* evalAuth, PropertyChangeEvent::Pointer event) :
  evalAuth(evalAuth), event(event)
{

}

void EvaluationAuthority::SafeServiceChangeRunnable::HandleException(
    const std::exception& exception)
{
  WorkbenchPlugin::Log(exception.what());
}

void EvaluationAuthority::SafeServiceChangeRunnable::Run()
{
  evalAuth->serviceEvents.propertyChange(event);
}

EvaluationAuthority::EvaluationAuthority() :
  notifying(0)
{

}

ObjectBool::Pointer EvaluationAuthority::ValueOf(bool result)
{
  return ObjectBool::Pointer(new ObjectBool(result));
}

std::vector<std::string> EvaluationAuthority::GetNames(
    IEvaluationReference::Pointer ref)
{
  ExpressionInfo info;
  ref->GetExpression()->CollectExpressionInfo(&info);
  std::set<std::string> allNames(info.GetAccessedVariableNames());
  if (info.HasDefaultVariableAccess())
  {
    allNames.insert(ISources::ACTIVE_CURRENT_SELECTION_NAME());
  }
  std::set<std::string> propNames(info.GetAccessedPropertyNames());
  allNames.insert(propNames.begin(), propNames.end());
  return std::vector<std::string>(allNames.begin(), allNames.end());
}

void EvaluationAuthority::RefsWithSameExpression(const std::vector<
    EvaluationReference::Pointer>& refs)
{
  unsigned int k = 0;
  while (k < refs.size() && !refs[k]->IsPostingChanges())
  {
    k++;
  }
  if (k >= refs.size())
  {
    return;
  }
  EvaluationReference::Pointer ref(refs[k]);
  bool oldValue = this->Evaluate(ref);
  ref->ClearResult();
  bool newValue = this->Evaluate(ref);
  if (oldValue != newValue)
  {
    this->FirePropertyChange(ref, this->ValueOf(oldValue), this->ValueOf(
        newValue));
  }
  for (k++; k < refs.size(); k++)
  {
    ref = refs[k];
    // this is not as expensive as it looks
    if (ref->IsPostingChanges())
    {
      oldValue = this->Evaluate(ref);
      if (oldValue != newValue)
      {
        ref->SetResult(newValue);
        this->FirePropertyChange(ref, this->ValueOf(oldValue), this->ValueOf(
            newValue));
      }
    }
  }
}

void EvaluationAuthority::StartSourceChange(
    const std::vector<std::string>& sourceNames)
{
  //    if (Policy.DEBUG_SOURCES) {
  //      Tracing.printTrace(COMPONENT, "start source changed: " //$NON-NLS-1$
  //          + Arrays.asList(sourceNames));
  //    }
  notifying++;
  if (notifying == 1)
  {
    ObjectBool::Pointer boolFalse(new ObjectBool(false));
    ObjectBool::Pointer boolTrue(new ObjectBool(true));
    this->FireServiceChange(IEvaluationService::PROP_NOTIFYING, boolFalse,
        boolTrue);
  }
}

void EvaluationAuthority::EndSourceChange(
    const std::vector<std::string>& sourceNames)
{
  //    if (Policy.DEBUG_SOURCES) {
  //      Tracing.printTrace(COMPONENT, "end source changed: " //$NON-NLS-1$
  //          + Arrays.asList(sourceNames));
  //    }
  if (notifying == 1)
  {
    ObjectBool::Pointer boolFalse(new ObjectBool(false));
    ObjectBool::Pointer boolTrue(new ObjectBool(true));
    this->FireServiceChange(IEvaluationService::PROP_NOTIFYING, boolTrue,
        boolFalse);
  }
  notifying--;
}

void EvaluationAuthority::FirePropertyChange(IEvaluationReference::Pointer ref,
    Object::Pointer oldValue, Object::Pointer newValue)
{
  PropertyChangeEvent::Pointer event(new PropertyChangeEvent(ref,
      ref->GetProperty(), oldValue, newValue));
  ref->GetListener()->PropertyChange(event);
}

void EvaluationAuthority::FireServiceChange(const std::string& property,
    Object::Pointer oldValue, Object::Pointer newValue)
{

  PropertyChangeEvent::Pointer event(new PropertyChangeEvent(Object::Pointer(
      this), property, oldValue, newValue));
  ISafeRunnable::Pointer safeRunnable(
      new SafeServiceChangeRunnable(this, event));
  SafeRunner::Run(safeRunnable);
}

void EvaluationAuthority::SourceChanged(int sourcePriority)
{
  // no-op, we want the other one
}

void EvaluationAuthority::SourceChanged(
    const std::vector<std::string>& sourceNames)
{
  this->StartSourceChange(sourceNames);
  try
  {
    // evaluations to recompute
    for (unsigned int i = 0; i < sourceNames.size(); i++)
    {
      const ExpressionsToCacheMap& cachesByExpression =
          cachesBySourceName[sourceNames[i]];
      if (!cachesByExpression.empty())
      {
        for (ExpressionsToCacheMap::ConstIterator iter =
            cachesByExpression.begin(); iter != cachesByExpression.end(); ++iter)
        {
          const CacheSet& expressionCache = iter->second;

          if (expressionCache.size() > 0)
          {
            std::vector<EvaluationReference::Pointer> refs(
                expressionCache.begin(), expressionCache.end());
            this->RefsWithSameExpression(refs);
          }
        }
      }
    }

    this->EndSourceChange(sourceNames);
  } catch (...)
  {
    this->EndSourceChange(sourceNames);
  }
}

void EvaluationAuthority::AddEvaluationListener(
    IEvaluationReference::Pointer ref)
{
  // we update the source priority bucket sort of activations.
  std::vector<std::string> sourceNames(this->GetNames(ref));
  for (unsigned int i = 0; i < sourceNames.size(); i++)
  {
    ExpressionsToCacheMap& cachesByExpression =
        cachesBySourceName[sourceNames[i]];
    Expression::Pointer expression(ref->GetExpression());
    cachesByExpression[expression].insert(ref.Cast<EvaluationReference> ());
  }

  bool result = this->Evaluate(ref);
  this->FirePropertyChange(ref, Object::Pointer(0), this->ValueOf(result));
}

void EvaluationAuthority::RemoveEvaluationListener(
    IEvaluationReference::Pointer ref)
{
  // Next we update the source priority bucket sort of activations.
  std::vector<std::string> sourceNames(this->GetNames(ref));
  for (unsigned int i = 0; i < sourceNames.size(); i++)
  {
    ExpressionsToCacheMap& cachesByExpression =
        cachesBySourceName[sourceNames[i]];

    CacheSet& caches = cachesByExpression[ref->GetExpression()];
    if (!caches.empty())
    {
      caches.erase(ref.Cast<EvaluationReference> ());
    }
  }
  bool result = this->Evaluate(ref);
  this->FirePropertyChange(ref, this->ValueOf(result), Object::Pointer(0));
}

void EvaluationAuthority::AddServiceListener(
    IPropertyChangeListener::Pointer listener)
{
  serviceEvents.AddListener(listener);
}

void EvaluationAuthority::RemoveServiceListener(
    IPropertyChangeListener::Pointer listener)
{
  serviceEvents.RemoveListener(listener);
}

Shell::Pointer EvaluationAuthority::GetActiveShell()
{
  return this->GetVariable(ISources::ACTIVE_SHELL_NAME()).Cast<Shell> ();
}

}
