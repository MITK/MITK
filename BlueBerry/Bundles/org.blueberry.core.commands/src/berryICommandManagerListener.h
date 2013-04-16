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

#include <berryObject.h>
#include <berryMacros.h>

namespace berry
{

class CommandManagerEvent;

/**
 * An instance of this interface can be used by clients to receive notification
 * of changes to one or more instances of <code>ICommandManager</code>.
 * <p>
 * This interface may be implemented by clients.
 * </p>
 *
 * @since 3.1
 * @see CommandManager#addCommandManagerListener(ICommandManagerListener)
 * @see CommandManager#removeCommandManagerListener(ICommandManagerListener)
 */
struct BERRY_COMMANDS ICommandManagerListener: public virtual Object
{

berryInterfaceMacro(ICommandManagerListener, berry)

struct Events {

    typedef Message1<const SmartPointer<const CommandManagerEvent> > Event;

    Event commandManagerChanged;

    void AddListener(ICommandManagerListener::Pointer listener);
    void RemoveListener(ICommandManagerListener::Pointer listener);

  private:
    typedef MessageDelegate1<ICommandManagerListener, const SmartPointer<const CommandManagerEvent> > Delegate;
  }  ;

  /**
   * Notifies that one or more properties of an instance of
   * <code>ICommandManager</code> have changed. Specific details are
   * described in the <code>CommandManagerEvent</code>.
   *
   * @param commandManagerEvent
   *            the commandManager event. Guaranteed not to be
   *            <code>null</code>.
   */
virtual  void CommandManagerChanged(const SmartPointer<const CommandManagerEvent> commandManagerEvent) = 0;
};

}

#endif /* BERRYICOMMANDMANAGERLISTENER_H_ */
