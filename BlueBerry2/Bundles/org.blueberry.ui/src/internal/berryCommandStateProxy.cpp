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

#include "berryCommandStateProxy.h"

namespace berry {

bool CommandStateProxy::LoadState() {
    return LoadState(false);
  }

  bool CommandStateProxy::LoadState(bool readPersistence) {
    if (!state) {
      try {
        state = (State) configurationElement
            .createExecutableExtension(stateAttributeName);
        state.setId(getId());
        configurationElement = null;

        // Try to load the persistent state, if possible.
        if (readPersistence && state instanceof PersistentState) {
          final PersistentState persistentState = (PersistentState) state;
          persistentState.setShouldPersist(true);
        }
        load(preferenceStore, preferenceKey);

        // Transfer the local listeners to the real state.
        final Object[] listenerArray = getListeners();
        for (int i = 0; i < listenerArray.length; i++) {
          state.addListener((IStateListener) listenerArray[i]);
        }
        clearListeners();

        return true;

      } catch (final ClassCastException e) {
        final String message = "The proxied state was the wrong class"; //$NON-NLS-1$
        final IStatus status = new Status(IStatus.ERROR,
            WorkbenchPlugin.PI_WORKBENCH, 0, message, e);
        WorkbenchPlugin.log(message, status);
        return false;

      } catch (const CoreException& e) {
        final String message = "The proxied state for '" + configurationElement.getAttribute(stateAttributeName) //$NON-NLS-1$
            + "' could not be loaded"; //$NON-NLS-1$
        IStatus status = new Status(IStatus.ERROR,
            WorkbenchPlugin.PI_WORKBENCH, 0, message, e);
        WorkbenchPlugin.log(message, status);
        return false;
      }
    }

    return true;
  }

CommandStateProxy::CommandStateProxy(const IConfigurationElement::Pointer configurationElement,
      const std::string& stateAttributeName,
      const IPreferenceStore::Pointer preferenceStore,
      const std::string& preferenceKey) {

    if (configurationElement == null) {
      throw new NullPointerException(
          "The configuration element backing a state proxy cannot be null"); //$NON-NLS-1$
    }

    if (stateAttributeName == null) {
      throw new NullPointerException(
          "The attribute containing the state class must be known"); //$NON-NLS-1$
    }

    this.configurationElement = configurationElement;
    this.stateAttributeName = stateAttributeName;
    this.preferenceKey = preferenceKey;
    this.preferenceStore = preferenceStore;
  }

  void CommandStateProxy::AddListener(const IStateListener::Pointer listener) {
    if (state == null) {
      addListenerObject(listener);
    } else {
      state.addListener(listener);
    }
  }

  void CommandStateProxy::Dispose() {
    if (state != null) {
      state.dispose();
      if (state instanceof PersistentState) {
        final PersistentState persistableState = (PersistentState) state;
        if (persistableState.shouldPersist() && preferenceStore != null
            && preferenceKey != null) {
          persistableState.save(preferenceStore, preferenceKey);
        }
      }
    }
  }

  Object::Pointer CommandStateProxy::GetValue() const {
    if (loadState()) {
      return state.getValue();
    }

    return null;
  }

  void CommandStateProxy::Load(const IPreferenceStore::Pointer store,
      const std::string& preferenceKey) {
    if (loadState() && state instanceof PersistentState) {
      final PersistentState persistableState = (PersistentState) state;
      if (persistableState.shouldPersist() && preferenceStore != null
          && preferenceKey != null) {
        persistableState.load(preferenceStore, preferenceKey);
      }
    }
  }

  void CommandStateProxy::RemoveListener(const IStateListener::Pointer listener) {
    if (state == null) {
      removeListenerObject(listener);
    } else {
      state.removeListener(listener);
    }
  }

  void CommandStateProxy::Save(const IPreferenceStore::Pointer store,
      const std::string& preferenceKey) {
    if (loadState() && state instanceof PersistentState) {
      ((PersistentState) state).save(store, preferenceKey);
    }
  }

   void CommandStateProxy::SetId(const std::string& id) {
    super.setId(id);
    if (state != null) {
      state.setId(id);
    }
  }

  void CommandStateProxy::SetShouldPersist(bool persisted) {
    if (loadState(persisted) && state instanceof PersistentState) {
      ((PersistentState) state).setShouldPersist(persisted);
    }
  }

  void CommandStateProxy::SetValue(Object::Pointer value) {
    if (loadState()) {
      state.setValue(value);
    }
  }

  bool CommandStateProxy::ShouldPersist() {
    if (loadState() && state instanceof PersistentState) {
      return ((PersistentState) state).shouldPersist();
    }

    return false;
  }

  std::string CommandStateProxy::ToString() const {
    if (state == null) {
      return configurationElement.getAttribute(stateAttributeName);
    }

    return state.toString();
  }

}
