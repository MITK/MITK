/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYCOMMANDSTATEPROXY_H
#define BERRYCOMMANDSTATEPROXY_H

#include "berryPersistentState.h"

namespace berry {

struct IConfigurationElement;
struct IPreferences;

/**
 * <p>
 * A proxy for handler state that has been defined in XML. This delays the class
 * loading until the state is really asked for information. Asking a proxy for
 * anything (except disposing, and adding and removing listeners) will cause the
 * proxy to instantiate the proxied handler.
 * </p>
 * <p>
 * Loading the proxied state will automatically cause it to load its value from
 * the preference store. Disposing of the state will cause it to persist its
 * value.
 * </p>
 * <p>
 * This class is not intended for use outside of the
 * <code>org.eclipse.ui.workbench</code> plug-in.
 * </p>
 */
class CommandStateProxy : public PersistentState
{

public:

  berryObjectMacro(berry::CommandStateProxy);

private:

  /**
   * The configuration element from which the state can be created. This value
   * will exist until the element is converted into a real class -- at which
   * point this value will be set to <code>null</code>.
   */
  SmartPointer<IConfigurationElement> configurationElement;

  /**
   * The key in the preference store to locate the persisted state.
   */
  const QString preferenceKey;

  /**
   * The preference store containing the persisted state, if any.
   */
  const SmartPointer<IPreferences> preferenceStore;

  /**
   * The real state. This value is <code>null</code> until the proxy is
   * forced to load the real state. At this point, the configuration element
   * is converted, nulled out, and this state gains a reference.
   */
  SmartPointer<State> state;

  /**
   * The name of the configuration element attribute which contains the
   * information necessary to instantiate the real state.
   */
  const QString stateAttributeName;

public:

  /**
   * Constructs a new instance of <code>HandlerState</code> with all the
   * information it needs to create the real state later.
   *
   * @param configurationElement
   *            The configuration element from which the real class can be
   *            loaded at run-time; must not be <code>null</code>.
   * @param stateAttributeName
   *            The name of the attribute or element containing the state
   *            executable extension; must not be <code>null</code>.
   * @param preferenceStore
   *            The preference store to which any persistent data should be
   *            written, and from which it should be loaded; may be
   *            <code>null</code>.
   * @param preferenceKey
   *            The key at which the persistent data is located within the
   *            preference store.
   */
  CommandStateProxy(const SmartPointer<IConfigurationElement>& configurationElement,
                    const QString& stateAttributeName,
                    const SmartPointer<IPreferences>& preferenceStore,
                    const QString& preferenceKey);

  void AddListener(IStateListener* listener) override;

  ~CommandStateProxy() override;

  Object::Pointer GetValue() const override;

  void Load(const SmartPointer<IPreferences>& store,
            const QString& preferenceKey) override;

  using State::RemoveListener;
  void RemoveListener(IStateListener* listener) override;

  void Save(const SmartPointer<IPreferences>& store,
            const QString& preferenceKey) override;

  void SetId(const QString& id) override;

  void SetShouldPersist(bool persisted) override;

  void SetValue(const Object::Pointer& value) override;

  bool ShouldPersist() override;

  QString ToString() const override;

private:

  /**
   * Loads the state, if possible. If the state is loaded, then the member
   * variables are updated accordingly and the state is told to load its value
   * from the preference store.
   *
   * @param readPersistence
   *            Whether the persistent state for this object should be read.
   * @return <code>true</code> if the state is now non-null;
   *         <code>false</code> otherwise.
   */
  bool LoadState(bool readPersistence = false);

};

}

#endif // BERRYCOMMANDSTATEPROXY_H
