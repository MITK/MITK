/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryCommandStateProxy.h"

#include "berryIConfigurationElement.h"
#include "berryStatus.h"
#include "berryPlatformUI.h"
#include "berryWorkbenchPlugin.h"
#include "berryIPreferences.h"

namespace berry {

CommandStateProxy::CommandStateProxy(const SmartPointer<IConfigurationElement>& configurationElement,
                                     const QString& stateAttributeName,
                                     const SmartPointer<IPreferences>& preferenceStore,
                                     const QString& preferenceKey)
  : configurationElement(configurationElement)
  , preferenceKey(preferenceKey)
  , preferenceStore(preferenceStore)
  , stateAttributeName(stateAttributeName)
{
  if (configurationElement.IsNull())
  {
    throw ctkInvalidArgumentException(
        "The configuration element backing a state proxy cannot be null");
  }

  if (stateAttributeName.isNull())
  {
    throw ctkInvalidArgumentException(
        "The attribute containing the state class must be known");
  }
}

void CommandStateProxy::AddListener(IStateListener* listener)
{
  if (state.IsNull())
  {
    this->AddListener(listener);
  }
  else
  {
    state->AddListener(listener);
  }
}

CommandStateProxy::~CommandStateProxy()
{
  if (state.IsNotNull())
  {
    if (PersistentState::Pointer persistableState = state.Cast<PersistentState>())
    {
      if (persistableState->ShouldPersist() && preferenceStore.IsNotNull()
          && !preferenceKey.isNull())
      {
        persistableState->Save(preferenceStore, preferenceKey);
      }
    }
  }
}

Object::Pointer CommandStateProxy::GetValue() const
{
  if (const_cast<CommandStateProxy*>(this)->LoadState())
  {
    return state->GetValue();
  }

  return Object::Pointer(nullptr);
}

void CommandStateProxy::Load(const SmartPointer<IPreferences>& /*store*/,
                             const QString& preferenceKey)
{
  if (LoadState())
  {
    if (PersistentState::Pointer persistableState = state.Cast<PersistentState>())
    {
      if (persistableState->ShouldPersist() && preferenceStore.IsNotNull()
          && !preferenceKey.isNull())
      {
      persistableState->Load(preferenceStore, preferenceKey);
      }
    }
  }
}

void CommandStateProxy::RemoveListener(IStateListener* listener)
{
  if (state.IsNull())
  {
    this->RemoveListener(listener);
  }
  else
  {
    state->RemoveListener(listener);
  }
}

void CommandStateProxy::Save(const SmartPointer<IPreferences>& store,
                             const QString& preferenceKey)
{
  if (LoadState())
  {
    if (PersistentState::Pointer persistableState = state.Cast<PersistentState>())
    {
      persistableState->Save(store, preferenceKey);
    }
  }
}

void CommandStateProxy::SetId(const QString& id)
{
  PersistentState::SetId(id);
  if (state.IsNotNull())
  {
    state->SetId(id);
  }
}

void CommandStateProxy::SetShouldPersist(bool persisted)
{
  if (LoadState(persisted))
  {
    if (PersistentState::Pointer persistableState = state.Cast<PersistentState>())
    {
      persistableState->SetShouldPersist(persisted);
    }
  }
}

void CommandStateProxy::SetValue(const Object::Pointer& value)
{
  if (LoadState())
  {
    state->SetValue(value);
  }
}

bool CommandStateProxy::ShouldPersist()
{
  if (LoadState())
  {
    if (PersistentState::Pointer persistableState = state.Cast<PersistentState>())
    {
      return persistableState->ShouldPersist();
    }
  }

  return false;
}

QString CommandStateProxy::ToString() const
{
  if (state.IsNull())
  {
    return configurationElement->GetAttribute(stateAttributeName);
  }

  return state->ToString();
}

bool CommandStateProxy::LoadState(bool readPersistence)
{
  if (state.IsNull())
  {
    try
    {
      state = configurationElement->CreateExecutableExtension<State>(stateAttributeName);

      if (state.IsNull())
      {
        const QString message = "The proxied state was the wrong class";
        const IStatus::Pointer status(new Status(IStatus::ERROR_TYPE,
                                                 PlatformUI::PLUGIN_ID(), 0, message, BERRY_STATUS_LOC));
        WorkbenchPlugin::Log(message, status);
        return false;
      }

      state->SetId(GetId());
      configurationElement = nullptr;

      // Try to load the persistent state, if possible.
      if (readPersistence)
      {
        if (PersistentState::Pointer persistentState = state.Cast<PersistentState>())
        {
          persistentState->SetShouldPersist(true);
        }
      }
      Load(preferenceStore, preferenceKey);

      // Transfer the local listeners to the real state.
      typedef IStateListener::Events::StateEvent::ListenerList ListenerListType;
      const ListenerListType listenerArray = this->stateEvents.stateChanged.GetListeners();
      for (auto i = listenerArray.begin();
           i != listenerArray.end(); ++i)
      {
        state->AddListener(*(*i));
        this->RemoveListener(*(*i));
      }

      return true;
    }
    catch (const CoreException& e)
    {
      const QString message = "The proxied state for '" + configurationElement->GetAttribute(stateAttributeName)
          + "' could not be loaded";
      IStatus::Pointer status(new Status(IStatus::ERROR_TYPE, PlatformUI::PLUGIN_ID(), 0,
                                         message, e, BERRY_STATUS_LOC));
      WorkbenchPlugin::Log(message, status);
      return false;
    }
  }

  return true;
}

}
