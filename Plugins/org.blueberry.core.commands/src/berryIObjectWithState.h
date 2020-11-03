/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIOBJECTWITHSTATE_H_
#define BERRYIOBJECTWITHSTATE_H_

#include "berryObject.h"

#include <org_blueberry_core_commands_Export.h>

namespace berry {

class State;

/**
 * <p>
 * An object that holds zero or more state objects. This state information can
 * be shared between different instances of <code>IObjectWithState</code>.
 * </p>
 * <p>
 * Clients may implement, but must not extend this interface.
 * </p>
 *
 * @see AbstractHandlerWithState
 */
struct BERRY_COMMANDS IObjectWithState : public virtual Object
{

  berryObjectMacro(berry::IObjectWithState);

  /**
   * Adds state to this object.
   *
   * @param id
   *            The identifier indicating the type of state being added; must
   *            not be <code>null</code>.
   * @param state
   *            The new state to add to this object; must not be
   *            <code>null</code>.
   */
  virtual void AddState(const QString& id, const SmartPointer<State>& state) = 0;

  /**
   * Gets the state with the given id.
   *
   * @param stateId
   *            The identifier of the state to retrieve; must not be
   *            <code>null</code>.
   * @return The state; may be <code>null</code> if there is no state with
   *         the given id.
   */
  virtual SmartPointer<State> GetState(const QString& stateId) const = 0;

  /**
   * Gets the identifiers for all of the state associated with this object.
   *
   * @return All of the state identifiers; may be empty, but never
   *         <code>null</code>.
   */
  virtual QList<QString> GetStateIds() const = 0;

  /**
   * Removes state from this object.
   *
   * @param stateId
   *            The id of the state to remove from this object; must not be
   *            <code>null</code>.
   */
  virtual void RemoveState(const QString& stateId) = 0;
};

}

#endif /*BERRYIOBJECTWITHSTATE_H_*/
