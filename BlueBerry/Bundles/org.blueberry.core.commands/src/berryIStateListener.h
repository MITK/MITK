/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef BERRYISTATELISTENER_H_
#define BERRYISTATELISTENER_H_

#include "berryObject.h"
#include "berryMacros.h"

#include <org_blueberry_core_commands_Export.h>

namespace berry {

class State;

/**
 * <p>
 * A listener to changes in some state.
 * </p>
 * <p>
 * Clients may implement, but must not extend this interface.
 * </p>
 *
 * @since 3.2
 */
struct BERRY_COMMANDS IStateListener : public virtual Object {

  berryInterfaceMacro(IStateListener, berry);

  struct Events {

    typedef Message2<SmartPointer<State>, Object::Pointer> StateEvent;

    StateEvent stateChanged;

    void AddListener(IStateListener::Pointer listener);
    void RemoveListener(IStateListener::Pointer listener);

  private:
    typedef MessageDelegate2<IStateListener, SmartPointer<State>, Object::Pointer> Delegate;
  };

  /**
   * Handles a change to the value in some state.
   *
   * @param state
   *            The state that has changed; never <code>null</code>. The
   *            value for this state has been updated to the new value.
   * @param oldValue
   *            The old value; may be anything.
   */
  virtual void HandleStateChange(SmartPointer<State> state, Object::Pointer oldValue) = 0;
};


}

#endif /* BERRYISTATELISTENER_H_ */
