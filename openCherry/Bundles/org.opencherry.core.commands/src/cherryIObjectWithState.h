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

#ifndef CHERRYIOBJECTWITHSTATE_H_
#define CHERRYIOBJECTWITHSTATE_H_

#include "cherryCommandsDll.h"
#include "cherryState.h"

#include <vector>

namespace cherry {

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
 * @since 3.2
 */
struct CHERRY_COMMANDS IObjectWithState : public virtual Object {

  cherryInterfaceMacro(IObjectWithState, cherry)

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
  virtual void AddState(const std::string& id, const State::Pointer state) = 0;

  /**
   * Gets the state with the given id.
   *
   * @param stateId
   *            The identifier of the state to retrieve; must not be
   *            <code>null</code>.
   * @return The state; may be <code>null</code> if there is no state with
   *         the given id.
   */
  virtual State::Pointer GetState(const std::string& stateId) const = 0;

  /**
   * Gets the identifiers for all of the state associated with this object.
   *
   * @return All of the state identifiers; may be empty, but never
   *         <code>null</code>.
   */
  virtual std::vector<std::string> GetStateIds() const = 0;

  /**
   * Removes state from this object.
   *
   * @param stateId
   *            The id of the state to remove from this object; must not be
   *            <code>null</code>.
   */
  virtual void RemoveState(const std::string& stateId) = 0;
};

}

#endif /*CHERRYIOBJECTWITHSTATE_H_*/
