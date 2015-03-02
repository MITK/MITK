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

#include <org_blueberry_core_commands_Export.h>

#include "berryMessage.h"

namespace berry {

template<class T> class SmartPointer;

class Object;
class State;

/**
 * <p>
 * A listener to changes in some state.
 * </p>
 * <p>
 * Clients may implement, but must not extend this interface.
 * </p>
 */
struct BERRY_COMMANDS IStateListener
{

  struct Events {

    typedef Message2<const SmartPointer<State>&, const SmartPointer<Object>&> StateEvent;

    StateEvent stateChanged;

    void AddListener(IStateListener* listener);
    void RemoveListener(IStateListener* listener);

    private:
    typedef MessageDelegate2<IStateListener, const SmartPointer<State>&, const SmartPointer<Object>&> Delegate;
  };

  virtual ~IStateListener();

  /**
   * Handles a change to the value in some state.
   *
   * @param state
   *            The state that has changed; never <code>null</code>. The
   *            value for this state has been updated to the new value.
   * @param oldValue
   *            The old value; may be anything.
   */
  virtual void HandleStateChange(const SmartPointer<State>& state,
                                 const SmartPointer<Object>& oldValue) = 0;
};


}

#endif /* BERRYISTATELISTENER_H_ */
