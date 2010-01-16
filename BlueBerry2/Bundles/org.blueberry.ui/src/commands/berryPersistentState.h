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


#ifndef BERRYPERSISTENTSTATE_H_
#define BERRYPERSISTENTSTATE_H_

#include <berryState.h>

#include "../preference/berryIPreferenceStore.h"

namespace berry {

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
class PersistentState : public State {

private:

  /**
   * Whether this state should be persisted.
   */
  bool persisted;


public:

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
  virtual void Load(const IPreferenceStore::Pointer store,
      const std::string& preferenceKey) = 0;

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
  virtual void Save(const IPreferenceStore::Pointer store,
      const std::string& preferenceKey) = 0;

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

};

}

#endif /* BERRYPERSISTENTSTATE_H_ */
