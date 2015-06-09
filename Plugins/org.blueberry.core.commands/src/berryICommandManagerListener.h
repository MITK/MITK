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

#ifndef BERRYICOMMANDMANAGERLISTENER_H_
#define BERRYICOMMANDMANAGERLISTENER_H_

#include <org_blueberry_core_commands_Export.h>

#include <berryMessage.h>

namespace berry
{

template<class T> class SmartPointer;

class CommandManagerEvent;

/**
 * An instance of this interface can be used by clients to receive notification
 * of changes to one or more instances of <code>ICommandManager</code>.
 * <p>
 * This interface may be implemented by clients.
 * </p>
 *
 * @see CommandManager#addCommandManagerListener(ICommandManagerListener)
 * @see CommandManager#removeCommandManagerListener(ICommandManagerListener)
 */
struct BERRY_COMMANDS ICommandManagerListener
{

  struct Events {

    typedef Message1<const SmartPointer<const CommandManagerEvent>& > Event;

    Event commandManagerChanged;

    void AddListener(ICommandManagerListener* listener);
    void RemoveListener(ICommandManagerListener* listener);

  private:
    typedef MessageDelegate1<ICommandManagerListener, const SmartPointer<const CommandManagerEvent>& > Delegate;
  };

  virtual ~ICommandManagerListener();

  /**
   * Notifies that one or more properties of an instance of
   * <code>ICommandManager</code> have changed. Specific details are
   * described in the <code>CommandManagerEvent</code>.
   *
   * @param commandManagerEvent
   *            the commandManager event. Guaranteed not to be
   *            <code>null</code>.
   */
  virtual  void CommandManagerChanged(const SmartPointer<const CommandManagerEvent>& commandManagerEvent) = 0;
};

}

#endif /* BERRYICOMMANDMANAGERLISTENER_H_ */
