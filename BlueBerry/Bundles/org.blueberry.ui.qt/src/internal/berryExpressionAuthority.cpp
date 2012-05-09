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


#include "berryExpressionAuthority.h"

#include <berryObjectVector.h>
#include "berryEvaluationContext.h"
#include "berryIEvaluationResultCache.h"

#include <berryISourceProvider.h>
#include <berryISources.h>
#include <berryIStructuredSelection.h>

namespace berry {

void ExpressionAuthority::SourceChanged(int sourcePriority, const QStringList& sourceNames)
{
  SourceChanged(sourcePriority);
  SourceChanged(sourceNames);
}

ExpressionAuthority::ExpressionAuthority()
  : context(new EvaluationContext(0, Object::Pointer(this)))
{
  // break smart-pointer self-cycle
  this->UnRegister(false);
  context->SetAllowPluginActivation(true);
  //context->AddVariable("org.blueberry.core.runtime.Platform", Platform.class);
}

bool ExpressionAuthority::Evaluate(const QList<SmartPointer<IEvaluationResultCache> >& collection) const
{
  foreach (IEvaluationResultCache::Pointer cache, collection)
  {
    if (Evaluate(cache))
    {
      return true;
    }
  }

  return false;
}

bool ExpressionAuthority::Evaluate(const SmartPointer<IEvaluationResultCache>& expression) const
{
  IEvaluationContext::Pointer contextWithDefaultVariable = GetCurrentState();
  return expression->Evaluate(contextWithDefaultVariable.GetPointer());
}

Object::Pointer ExpressionAuthority::GetVariable(const QString& name) const
{
  return context->GetVariable(name.toStdString());
}

void ExpressionAuthority::ChangeVariable(const QString& name, const Object::Pointer& value)
{
  if (value.IsNull())
  {
    context->RemoveVariable(name.toStdString());
  }
  else
  {
    context->AddVariable(name.toStdString(), value);
  }
}

void ExpressionAuthority::SourceChanged(const QStringList& sourceNames)
{
  // this is a no-op, since we're late in the game
}

void ExpressionAuthority::UpdateCurrentState()
{
  foreach (ISourceProvider::Pointer provider, providers)
  {
    ISourceProvider::StateMapType currentState = provider->GetCurrentState();
    QHashIterator<QString,Object::Pointer> variableItr(currentState);
    while (variableItr.hasNext())
    {
      variableItr.next();
      const QString variableName = variableItr.key();
      const Object::Pointer variableValue = variableItr.value();
      /*
       * Bug 84056. If we update the active workbench window, then we
       * risk falling back to that shell when the active shell has
       * registered as "none".
       */
      if (!variableName.isNull() &&
          ISources::ACTIVE_WORKBENCH_WINDOW_SHELL_NAME() != variableName)
      {
        ChangeVariable(variableName, variableValue);
      }
    }
  }
}

void ExpressionAuthority::UpdateEvaluationContext(const QString& name, const Object::Pointer& value)
{
  if (!name.isNull())
  {
    ChangeVariable(name, value);
  }
}

void ExpressionAuthority::AddSourceProvider(const SmartPointer<ISourceProvider>& provider)
{
  provider->AddSourceProviderListener(this);
  providers.push_back(provider);

  // Update the current state.
  ISourceProvider::StateMapType currentState = provider->GetCurrentState();
  QHashIterator<QString,Object::Pointer> variableItr(currentState);
  while (variableItr.hasNext())
  {
    variableItr.next();
    const QString variableName = variableItr.key();
    const Object::Pointer variableValue = variableItr.value();

    /*
     * Bug 84056. If we update the active workbench window, then we risk
     * falling back to that shell when the active shell has registered
     * as "none".
     */
    if (!variableName.isNull()
        && ISources::ACTIVE_WORKBENCH_WINDOW_SHELL_NAME() != variableName)
    {
      ChangeVariable(variableName, variableValue);
    }
  }
  SourceChanged(0, currentState);
}

ExpressionAuthority::~ExpressionAuthority()
{
  this->Dispose();
}

void ExpressionAuthority::Dispose()
{
  foreach (ISourceProvider::Pointer provider, providers)
  {
    provider->RemoveSourceProviderListener(this);
  }
  providers.clear();

  // prevent double delete
  this->Register();
  context = 0;
  this->UnRegister(false);
}

SmartPointer<IEvaluationContext> ExpressionAuthority::GetCurrentState() const
{
  if (currentState.IsNull())
  {
    const Object::Pointer defaultVariable =
        context->GetVariable(ISources::ACTIVE_CURRENT_SELECTION_NAME().toStdString());
    IEvaluationContext::Pointer contextWithDefaultVariable;
    if (IStructuredSelection::Pointer selection = defaultVariable.Cast<IStructuredSelection>())
    {
      contextWithDefaultVariable = new EvaluationContext(context.GetPointer(),
                                                         selection->ToVector());
    }
    else if (defaultVariable.Cast<ISelection>() &&
             !defaultVariable.Cast<ISelection>()->IsEmpty())
    {
      ObjectVector<Object::Pointer>::Pointer defaultObj(new ObjectVector<Object::Pointer>());
      defaultObj->push_back(defaultVariable);
      contextWithDefaultVariable = new EvaluationContext(context.GetPointer(),
                                                         defaultObj);
    }
    else
    {
      ObjectVector<Object::Pointer>::Pointer defaultObj(new ObjectVector<Object::Pointer>());
      contextWithDefaultVariable = new EvaluationContext(context.GetPointer(), defaultObj);
    }
    currentState = contextWithDefaultVariable;
  }

  return currentState;
}

void ExpressionAuthority::RemoveSourceProvider(const SmartPointer<ISourceProvider>& provider)
{
  provider->RemoveSourceProviderListener(this);
  providers.removeAll(provider);

  ISourceProvider::StateMapType currentState = provider->GetCurrentState();
  QHashIterator<QString,Object::Pointer> variableItr(currentState);
  while (variableItr.hasNext())
  {
    variableItr.next();
    const QString variableName = variableItr.key();
    ChangeVariable(variableName, Object::Pointer(0));
  }
}

void ExpressionAuthority::SourceChanged(int sourcePriority,
                                        const QHash<QString, Object::Pointer> &sourceValuesByName)
{
  // If the selection has changed, invalidate the current state.
  if (sourceValuesByName.contains(ISources::ACTIVE_CURRENT_SELECTION_NAME()))
  {
    currentState = 0;
  }

  QHashIterator<QString, Object::Pointer> entryItr(sourceValuesByName);
  while (entryItr.hasNext())
  {
    entryItr.next();
    const QString sourceName = entryItr.key();
    const Object::Pointer sourceValue = entryItr.value();
    UpdateEvaluationContext(sourceName, sourceValue);
  }
  SourceChanged(sourcePriority, sourceValuesByName.keys());
}

void ExpressionAuthority::SourceChanged(int sourcePriority, const QString &sourceName,
                   Object::Pointer sourceValue)
{
  // If the selection has changed, invalidate the current state.
  if (ISources::ACTIVE_CURRENT_SELECTION_NAME() == sourceName)
  {
    currentState = 0;
  }

  UpdateEvaluationContext(sourceName, sourceValue);
  SourceChanged(sourcePriority, QStringList(sourceName));
}

}
