/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYPERSISTENTSTATE_H
#define BERRYPERSISTENTSTATE_H

#include "berryState.h"

namespace berry {

struct IPreferences;

/**
 * <p>
 * This is a state that can be made persistent. A state is persisted to a
 * preference store.
 * </p>
 * <p>
 * Clients may extend this class.
 * </p>
 *
 * @since 3.2
 */
class PersistentState : public State
{

public:

  berryObjectMacro(berry::PersistentState);

  PersistentState();

  /**
   * Loads this state from the preference store, given the location at which
   * to look. This method must be symmetric with a call to
   * {@link #save(IPreferenceStore, String)}.
   *
   * @param store
   *            The store from which to read; must not be <code>null</code>.
   * @param preferenceKey
   *            The key at which the state is stored; must not be
   *            <code>null</code>.
   */
  virtual void Load(const SmartPointer<IPreferences>& store,
                    const QString& preferenceKey) = 0;

  /**
   * Saves this state to the preference store, given the location at which to
   * write. This method must be symmetric with a call to
   * {@link #load(IPreferenceStore, String)}.
   *
   * @param store
   *            The store to which the state should be written; must not be
   *            <code>null</code>.
   * @param preferenceKey
   *            The key at which the state should be stored; must not be
   *            <code>null</code>.
   */
  virtual void Save(const SmartPointer<IPreferences>& store,
                    const QString& preferenceKey) = 0;

  /**
   * Sets whether this state should be persisted.
   *
   * @param persisted
   *            Whether this state should be persisted.
   */
  virtual void SetShouldPersist(bool persisted);

  /**
   * Whether this state should be persisted. Subclasses should check this
   * method before loading or saving.
   *
   * @return <code>true</code> if this state should be persisted;
   *         <code>false</code> otherwise.
   */
  virtual bool ShouldPersist();

private:

  /**
   * Whether this state should be persisted.
   */
  bool persisted;
};

}

#endif // BERRYPERSISTENTSTATE_H
