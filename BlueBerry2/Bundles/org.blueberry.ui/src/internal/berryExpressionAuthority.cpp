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

#include "berryExpressionAuthority.h"
#include "../berryISources.h"
#include "../berryIStructuredSelection.h"

#include <osgi/framework/ObjectString.h>
#include <osgi/framework/ObjectVector.h>
#include <berryPlatform.h>
#include <berryEvaluationContext.h>

namespace berry
{

 void ExpressionAuthority::SourceChanged(int sourcePriority, const std::vector<std::string>& sourceNames) {
    this->SourceChanged(sourcePriority);
    this->SourceChanged(sourceNames);
  }

  ExpressionAuthority::ExpressionAuthority() {
    context = new EvaluationContext(IEvaluationContext::Pointer(0), Object::Pointer(this));
    context->SetAllowPluginActivation(true);
    ObjectString::Pointer var(new ObjectString(Platform::GetStaticClassName()));
    context->AddVariable("org.blueberry.core.runtime.Platform", var); //$NON-NLS-1$
    // this is not as useful as it appears
    // context.addVariable("java.lang.System", System.class); //$NON-NLS-1$
  }

  bool ExpressionAuthority::Evaluate(const std::vector<IEvaluationResultCache::Pointer>& collection) {
    for(std::vector<IEvaluationResultCache::Pointer>::const_iterator iter = collection.begin();
          iter != collection.end(); ++iter)
    {
      if (this->Evaluate(*iter)) {
        return true;
      }
    }

    return false;
  }

  bool ExpressionAuthority::Evaluate(IEvaluationResultCache::Pointer expression) {
    IEvaluationContext::Pointer contextWithDefaultVariable(this->GetCurrentState());
    return expression->Evaluate(contextWithDefaultVariable);
  }

  Object::Pointer ExpressionAuthority::GetVariable(const std::string& name) {
    return context->GetVariable(name);
  }

  void ExpressionAuthority::ChangeVariable(const std::string& name, Object::Pointer value) {
    if (!value) {
      context->RemoveVariable(name);
    } else {
      context->AddVariable(name, value);
    }
  }

  void ExpressionAuthority::SourceChanged(const std::vector<std::string>& sourceNames) {
    // this is a no-op, since we're late in the game
  }

  void ExpressionAuthority::UpdateCurrentState() {
    for(std::vector<ISourceProvider::Pointer>::iterator provider = providers.begin();
        provider != providers.end(); ++provider)
    {
      ISourceProvider::StateMapType currentState = (*provider)->GetCurrentState();
      for(ISourceProvider::StateMapType::iterator iter = currentState.begin();
          iter != currentState.end(); ++iter)
      {
        std::string variableName(iter->first);
        Object::Pointer variableValue(iter->second);
        /*
         * Bug 84056. If we update the active workbench window, then we
         * risk falling back to that shell when the active shell has
         * registered as "none".
         */
        if (!variableName.empty()
            && (ISources::ACTIVE_WORKBENCH_WINDOW_SHELL_NAME()
                != variableName)) {
          this->ChangeVariable(variableName, variableValue);
        }
      }
    }
  }

  void ExpressionAuthority::UpdateEvaluationContext(const std::string& name, Object::Pointer value) {
    if (!name.empty()) {
      this->ChangeVariable(name, value);
    }
  }

  void ExpressionAuthority::AddSourceProvider(ISourceProvider::Pointer provider) {
    provider->AddSourceProviderListener(ISourceProviderListener::Pointer(this));
    providers.push_back(provider);

    // Update the current state.
    ISourceProvider::StateMapType currentState = provider->GetCurrentState();
      for(ISourceProvider::StateMapType::iterator iter = currentState.begin();
          iter != currentState.end(); ++iter)
      {
        std::string variableName(iter->first);
        Object::Pointer variableValue(iter->second);
        /*
         * Bug 84056. If we update the active workbench window, then we
         * risk falling back to that shell when the active shell has
         * registered as "none".
         */
        if (!variableName.empty()
            && (ISources::ACTIVE_WORKBENCH_WINDOW_SHELL_NAME()
                != variableName)) {
          this->ChangeVariable(variableName, variableValue);
        }
      }
  }

  void ExpressionAuthority::Dispose() {
    for(std::vector<ISourceProvider::Pointer>::iterator iter = providers.begin();
        iter != providers.end(); ++iter)
    {
      (*iter)->RemoveSourceProviderListener(ISourceProviderListener::Pointer(this));
    }

    providers.clear();
  }

  IEvaluationContext::Pointer ExpressionAuthority::GetCurrentState() {
    if (!currentState) {
      Object::Pointer defaultVariable = context
          ->GetVariable(ISources::ACTIVE_CURRENT_SELECTION_NAME());
      IEvaluationContext::Pointer contextWithDefaultVariable;
      if (IStructuredSelection::Pointer selection = defaultVariable.Cast<IStructuredSelection>()) {
        contextWithDefaultVariable = IEvaluationContext::Pointer(new EvaluationContext(context,
            selection->ToVector()));
      } else if (ISelection::Pointer selection = defaultVariable.Cast<ISelection>())
      {
          if(!selection->IsEmpty())
          {
            ObjectVector<Object::Pointer>::Pointer coll(new ObjectVector<Object::Pointer>());
            coll->push_back(defaultVariable);
            contextWithDefaultVariable = new EvaluationContext(context, coll);
          }
      }
      else {
        Object::Pointer coll(new ObjectVector<Object::Pointer>());
        contextWithDefaultVariable = new EvaluationContext(context, coll);
      }
      currentState = contextWithDefaultVariable;
    }

    return currentState;
  }

  void ExpressionAuthority::RemoveSourceProvider(ISourceProvider::Pointer provider) {
    provider->RemoveSourceProviderListener(ISourceProviderListener::Pointer(this));
    providers.erase(std::find(providers.begin(), providers.end(), provider));

    ISourceProvider::StateMapType currentState = provider->GetCurrentState();
    for (ISourceProvider::StateMapType::iterator iter = currentState.begin();
         iter != currentState.end(); ++iter)
    {
      std::string variableName = iter->first;
      this->ChangeVariable(variableName, Object::Pointer(0));
    }
  }

  void ExpressionAuthority::SourceChanged(int sourcePriority,
      const std::map<std::string, Object::Pointer>& sourceValuesByName) {
    // If the selection has changed, invalidate the current state.
    if (sourceValuesByName
        .find(ISources::ACTIVE_CURRENT_SELECTION_NAME()) != sourceValuesByName.end()) {
      currentState = 0;
    }

    std::vector<std::string> sourceValues;
    for (std::map<std::string, Object::Pointer>::const_iterator iter = sourceValuesByName.begin();
         iter != sourceValuesByName.end(); ++iter)
    {
      std::string sourceName = iter->first;
      Object::Pointer sourceValue = iter->second;
      sourceValues.push_back(sourceName);
      this->UpdateEvaluationContext(sourceName, sourceValue);
    }
    this->SourceChanged(sourcePriority, sourceValues);
  }

  void ExpressionAuthority::SourceChanged(int sourcePriority,
      const std::string& sourceName, Object::Pointer sourceValue) {
    // If the selection has changed, invalidate the current state.
    if (ISources::ACTIVE_CURRENT_SELECTION_NAME() == sourceName) {
      currentState = 0;
    }

    this->UpdateEvaluationContext(sourceName, sourceValue);
    std::vector<std::string> sourceNames;
    sourceNames.push_back(sourceName);
    this->SourceChanged(sourcePriority, sourceNames);
  }


}
