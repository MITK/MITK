/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYICOMMANDLISTENER_H_
#define BERRYICOMMANDLISTENER_H_

#include <berryMessage.h>

#include <org_blueberry_core_commands_Export.h>

namespace berry {

template<class T> class SmartPointer;

class CommandEvent;

/**
 * An instance of this interface can be used by clients to receive notification
 * of changes to one or more instances of <code>Command</code>.
 * <p>
 * This interface may be implemented by clients.
 * </p>
 *
 * @see Command#addCommandListener(ICommandListener)
 * @see Command#removeCommandListener(ICommandListener)
 */
struct BERRY_COMMANDS ICommandListener
{

  struct Events {

    typedef Message1<const SmartPointer<const CommandEvent>&> Event;

    Event commandChanged;

    void AddListener(ICommandListener* listener);
    void RemoveListener(ICommandListener* listener);

  private:
    typedef MessageDelegate1<ICommandListener, const SmartPointer<const CommandEvent>&> Delegate;
  };

  virtual ~ICommandListener();

  /**
   * Notifies that one or more properties of an instance of
   * <code>Command</code> have changed. Specific details are described in
   * the <code>CommandEvent</code>.
   *
   * @param commandEvent
   *            the command event. Guaranteed not to be <code>null</code>.
   */
  virtual void CommandChanged(const SmartPointer<const CommandEvent>& commandEvent) = 0;
};

}

#endif /* BERRYICOMMANDLISTENER_H_ */
