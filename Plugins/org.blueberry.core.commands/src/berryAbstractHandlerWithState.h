/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYABSTRACTHANDLERWITHSTATE_H
#define BERRYABSTRACTHANDLERWITHSTATE_H

#include "berryAbstractHandler.h"
#include "berryIObjectWithState.h"
#include "berryIStateListener.h"

#include <org_blueberry_core_commands_Export.h>

namespace berry {

class State;

/**
 * <p>
 * An abstract implementation of {@link IObjectWithState}. This provides basic
 * handling for adding and remove state. When state is added, the handler
 * attaches itself as a listener and fire a handleStateChange event to notify
 * this handler. When state is removed, the handler removes itself as a
 * listener.
 * </p>
 * <p>
 * Clients may extend this class.
 * </p>
 */
class BERRY_COMMANDS AbstractHandlerWithState : public AbstractHandler, public IObjectWithState,
    protected IStateListener
{

private:

  /**
   * The map of states currently held by this handler. If this handler has no
   * state (generally, when inactive), then this will be <code>null</code>.
   */
  QHash<QString,SmartPointer<State> > states;

public:

  berryObjectMacro(berry::AbstractHandlerWithState);

  /**
   * <p>
   * Adds a state to this handler. This will add this handler as a listener to
   * the state, and then fire a handleStateChange so that the handler can
   * respond to the incoming state.
   * </p>
   * <p>
   * Clients may extend this method, but they should call this super method
   * first before doing anything else.
   * </p>
   *
   * @param stateId
   *            The identifier indicating the type of state being added; must
   *            not be <code>null</code>.
   * @param state
   *            The state to add; must not be <code>null</code>.
   */
  void AddState(const QString& stateId, const SmartPointer<State>& state) override;

  SmartPointer<State> GetState(const QString& stateId) const override;

  QList<QString> GetStateIds() const override;

  /**
   * <p>
   * Removes a state from this handler. This will remove this handler as a
   * listener to the state. No event is fired to notify the handler of this
   * change.
   * </p>
   * <p>
   * Clients may extend this method, but they should call this super method
   * first before doing anything else.
   * </p>
   *
   * @param stateId
   *            The identifier of the state to remove; must not be
   *            <code>null</code>.
   */
  void RemoveState(const QString& stateId) override;
};

}

#endif // BERRYABSTRACTHANDLERWITHSTATE_H
