/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYSTATE_H_
#define BERRYSTATE_H_

#include <berryMacros.h>
#include <berryObject.h>
#include <org_blueberry_core_commands_Export.h>

#include "berryIStateListener.h"

namespace berry {

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
class BERRY_COMMANDS State : public Object {

public:

  berryObjectMacro(State);

  /**
   * Adds a listener to changes for this state.
   *
   * @param listener
   *            The listener to add; must not be <code>null</code>.
   */
  virtual void AddListener(IStateListener* listener);

  void AddListener(const IStateListener::Events::StateEvent::AbstractDelegate& delegate);

  /**
   * Returns the identifier for this state.
   *
   * @return The id; may be <code>null</code>.
   */
   QString GetId() const;

  /**
   * The current value associated with this state. This can be any type of
   * object, but implementations will usually restrict this value to a
   * particular type.
   *
   * @return The current value; may be anything.
   */

  virtual Object::Pointer GetValue() const;

  /**
   * Removes a listener to changes from this state.
   *
   * @param listener
   *            The listener to remove; must not be <code>null</code>.
   */
  virtual void RemoveListener(IStateListener* listener);

  void RemoveListener(const IStateListener::Events::StateEvent::AbstractDelegate& delegate);

  /**
   * Sets the identifier for this object.  This method should only be called
   * by the command framework.  Clients should not call this method.
   *
   * @param id
   *            The id; must not be <code>null</code>.
   */
  virtual void SetId(const QString& id);

  /**
   * Sets the value for this state object.
   *
   * @param value
   *            The value to set; may be anything.
   */
  virtual void SetValue(const Object::Pointer& value);


protected:

  /**
   * Notifies listeners to this state that it has changed in some way.
   *
   * @param oldValue
   *            The old value; may be anything.
   */
  void FireStateChanged(const Object::Pointer& oldValue);

  IStateListener::Events stateEvents;

private:

  /**
   * The identifier of the state; may be <code>null</code> if it has not
   * been initialized.
   */
  QString id;

  /**
   * The value held by this state; may be anything at all.
   */
  Object::Pointer value;

};

}

Q_DECLARE_INTERFACE(berry::State, "org.blueberry.core.commands.State")

#endif /*BERRYSTATE_H_*/
