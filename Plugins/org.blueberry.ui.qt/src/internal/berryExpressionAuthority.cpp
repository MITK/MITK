/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "berryExpressionAuthority.h"

#include <berryObjectList.h>
#include <berryObjectTypeInfo.h>
#include <berryPlatform.h>

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
  : context(new EvaluationContext(nullptr, Object::Pointer(this)))
{
  // break smart-pointer self-cycle
  this->UnRegister(false);
  context->SetAllowPluginActivation(true);
  Object::Pointer platformVariable(new ObjectTypeInfo(Reflection::TypeInfo::New<Platform>()));
  context->AddVariable("org.blueberry.core.runtime.Platform", platformVariable);
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

Object::ConstPointer ExpressionAuthority::GetVariable(const QString& name) const
{
  return context->GetVariable(name);
}

void ExpressionAuthority::ChangeVariable(const QString& name, const Object::ConstPointer& value)
{
  if (value.IsNull())
  {
    context->RemoveVariable(name);
  }
  else
  {
    context->AddVariable(name, value);
  }
}

void ExpressionAuthority::SourceChanged(const QStringList& /*sourceNames*/)
{
  // this is a no-op, since we're late in the game
}

void ExpressionAuthority::UpdateCurrentState()
{
  foreach (ISourceProvider::Pointer provider, providers)
  {
    ISourceProvider::StateMapType currentState = provider->GetCurrentState();
    QHashIterator<QString,Object::ConstPointer> variableItr(currentState);
    while (variableItr.hasNext())
    {
      variableItr.next();
      const QString variableName = variableItr.key();
      const Object::ConstPointer variableValue = variableItr.value();
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

void ExpressionAuthority::UpdateEvaluationContext(const QString& name, const Object::ConstPointer& value)
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
  QHashIterator<QString,Object::ConstPointer> variableItr(currentState);
  while (variableItr.hasNext())
  {
    variableItr.next();
    const QString variableName = variableItr.key();
    const Object::ConstPointer variableValue = variableItr.value();

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

  if (context.IsNotNull())
  {
    // prevent double delete
    this->Register();
    context = nullptr;
    //this->UnRegister(false);
  }
}

SmartPointer<IEvaluationContext> ExpressionAuthority::GetCurrentState() const
{
  if (currentState.IsNull())
  {
    const Object::ConstPointer defaultVariable =
        context->GetVariable(ISources::ACTIVE_CURRENT_SELECTION_NAME());
    IEvaluationContext::Pointer contextWithDefaultVariable;
    if (IStructuredSelection::ConstPointer selection = defaultVariable.Cast<const IStructuredSelection>())
    {
      contextWithDefaultVariable = new EvaluationContext(context.GetPointer(),
                                                         selection->ToVector());
    }
    else if (defaultVariable.Cast<const ISelection>() &&
             !defaultVariable.Cast<const ISelection>()->IsEmpty())
    {
      ObjectList<Object::ConstPointer>::Pointer defaultObj(new ObjectList<Object::ConstPointer>());
      defaultObj->push_back(defaultVariable);
      contextWithDefaultVariable = new EvaluationContext(context.GetPointer(),
                                                         defaultObj);
    }
    else
    {
      ObjectList<Object::ConstPointer>::Pointer defaultObj(new ObjectList<Object::ConstPointer>());
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
  QHashIterator<QString,Object::ConstPointer> variableItr(currentState);
  while (variableItr.hasNext())
  {
    variableItr.next();
    const QString variableName = variableItr.key();
    ChangeVariable(variableName, Object::Pointer(nullptr));
  }
}

void ExpressionAuthority::SourceChanged(int sourcePriority,
                                        const QHash<QString, Object::ConstPointer> &sourceValuesByName)
{
  // If the selection has changed, invalidate the current state.
  if (sourceValuesByName.contains(ISources::ACTIVE_CURRENT_SELECTION_NAME()))
  {
    currentState = nullptr;
  }

  QHashIterator<QString, Object::ConstPointer> entryItr(sourceValuesByName);
  while (entryItr.hasNext())
  {
    entryItr.next();
    const QString sourceName = entryItr.key();
    const Object::ConstPointer sourceValue = entryItr.value();
    UpdateEvaluationContext(sourceName, sourceValue);
  }
  SourceChanged(sourcePriority, sourceValuesByName.keys());
}

void ExpressionAuthority::SourceChanged(int sourcePriority, const QString &sourceName,
                   Object::ConstPointer sourceValue)
{
  // If the selection has changed, invalidate the current state.
  if (ISources::ACTIVE_CURRENT_SELECTION_NAME() == sourceName)
  {
    currentState = nullptr;
  }

  UpdateEvaluationContext(sourceName, sourceValue);
  SourceChanged(sourcePriority, QStringList(sourceName));
}

}
