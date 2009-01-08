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

#ifndef CHERRYSTATE_H_
#define CHERRYSTATE_H_

#include <cherryMacros.h>
#include <cherryObject.h>
#include "cherryCommandsDll.h"

namespace cherry {

/**
 * <p>
 * A piece of state information that can be shared between objects, and might be
 * persisted between sessions. This can be used for commands that toggle between
 * two states and wish to pass this state information between different
 * handlers.
 * </p>
 * <p>
 * This state object can either be used as a single state object shared between
 * several commands, or one state object per command -- depending on the needs
 * of the application.
 * </p>
 * <p>
 * Clients may instantiate or extend this class.
 * </p>
 *
 * @since 3.2
 */
class CHERRY_COMMANDS State : public Object { // extends EventManager {

public:

  cherryClassMacro(State)

private:

  /**
   * The identifier of the state; may be <code>null</code> if it has not
   * been initialized.
   */
  std::string id;

  /**
   * The value held by this state; may be anything at all.
   */
  Object::Pointer value;


public:

  /**
   * Adds a listener to changes for this state.
   *
   * @param listener
   *            The listener to add; must not be <code>null</code>.
   */
//  void AddListener(final IStateListener listener) {
//    addListenerObject(listener);
//  }


protected:

  /**
   * Notifies listeners to this state that it has changed in some way.
   *
   * @param oldValue
   *            The old value; may be anything.
   */
//  final void fireStateChanged(final Object oldValue) {
//    final Object[] listeners = getListeners();
//    for (int i = 0; i < listeners.length; i++) {
//      final IStateListener listener = (IStateListener) listeners[i];
//      listener.handleStateChange(this, oldValue);
//    }
//  }


public:

  /**
   * Returns the identifier for this state.
   *
   * @return The id; may be <code>null</code>.
   */
   std::string GetId();

  /**
   * The current value associated with this state. This can be any type of
   * object, but implementations will usually restrict this value to a
   * particular type.
   *
   * @return The current value; may be anything.
   */

  Object::Pointer GetValue();

  /**
   * Removes a listener to changes from this state.
   *
   * @param listener
   *            The listener to remove; must not be <code>null</code>.
   */
//  void RemoveListener(final IStateListener listener) {
//    removeListenerObject(listener);
//  }

  /**
   * Sets the identifier for this object.  This method should only be called
   * by the command framework.  Clients should not call this method.
   *
   * @param id
   *            The id; must not be <code>null</code>.
   */
  void SetId(const std::string& id);

  /**
   * Sets the value for this state object.
   *
   * @param value
   *            The value to set; may be anything.
   */
  void SetValue(const Object::Pointer value);
};

}

#endif /*CHERRYSTATE_H_*/
