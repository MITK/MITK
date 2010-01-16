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


#ifndef BERRYCOMMANDSTATEPROXY_H_
#define BERRYCOMMANDSTATEPROXY_H_

#include "../commands/berryPersistentState.h"

namespace berry {

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
 * <code>org.blueberry.ui</code> plug-in.
 * </p>
 *
 * @since 3.2
 */
class CommandStateProxy : public PersistentState {

private:

  /**
   * The configuration element from which the state can be created. This value
   * will exist until the element is converted into a real class -- at which
   * point this value will be set to <code>null</code>.
   */
  const IConfigurationElement::Pointer configurationElement;

  /**
   * The key in the preference store to locate the persisted state.
   */
  std::string preferenceKey;

  /**
   * The preference store containing the persisted state, if any.
   */
  IPreferenceStore::Pointer preferenceStore;

  /**
   * The real state. This value is <code>null</code> until the proxy is
   * forced to load the real state. At this point, the configuration element
   * is converted, nulled out, and this state gains a reference.
   */
   State::Pointer state;

  /**
   * The name of the configuration element attribute which contains the
   * information necessary to instantiate the real state.
   */
  const std::string stateAttributeName;

  /**
   * Loads the state, if possible. If the state is loaded, then the member
   * variables are updated accordingly and the state is told to load its value
   * from the preference store.
   *
   * @return <code>true</code> if the state is now non-null;
   *         <code>false</code> otherwise.
   */
  bool LoadState();

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
  bool LoadState(bool readPersistence);


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
  CommandStateProxy(const IConfigurationElement::Pointer configurationElement,
      const std::string& stateAttributeName,
      const IPreferenceStore::Pointer preferenceStore,
      const std::string& preferenceKey);

  void AddListener(const IStateListener::Pointer listener);

  void Dispose();

  Object::Pointer GetValue() const;

  void Load(const IPreferenceStore::Pointer store,
      const std::string& preferenceKey);

  void RemoveListener(const IStateListener::Pointer listener);

  void Save(const IPreferenceStore::Pointer store,
      const std::string& preferenceKey);

   void SetId(const std::string& id);

  void SetShouldPersist(bool persisted);

  void SetValue(Object::Pointer value);

  bool ShouldPersist();

  std::string ToString() const;
};

}

#endif /* BERRYCOMMANDSTATEPROXY_H_ */
