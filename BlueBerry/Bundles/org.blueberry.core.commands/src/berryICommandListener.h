/*=========================================================================

 Program:   BlueBerry Platform
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


#ifndef BERRYICOMMANDLISTENER_H_
#define BERRYICOMMANDLISTENER_H_

#include <berryObject.h>
#include <berryMacros.h>
#include <berryMessage.h>

namespace berry {

class CommandEvent;

/**
 * An instance of this interface can be used by clients to receive notification
 * of changes to one or more instances of <code>Command</code>.
 * <p>
 * This interface may be implemented by clients.
 * </p>
 *
 * @since 3.1
 * @see Command#addCommandListener(ICommandListener)
 * @see Command#removeCommandListener(ICommandListener)
 */
struct ICommandListener : public virtual Object {

  berryInterfaceMacro(ICommandListener, berry)

  struct Events {

    typedef Message1<const SmartPointer<const CommandEvent> > Event;

    Event commandChanged;

    void AddListener(ICommandListener::Pointer listener);
    void RemoveListener(ICommandListener::Pointer listener);

  private:
    typedef MessageDelegate1<ICommandListener, const SmartPointer<const CommandEvent> > Delegate;
  };

  /**
   * Notifies that one or more properties of an instance of
   * <code>Command</code> have changed. Specific details are described in
   * the <code>CommandEvent</code>.
   *
   * @param commandEvent
   *            the command event. Guaranteed not to be <code>null</code>.
   */
  virtual void CommandChanged(const SmartPointer<const CommandEvent> commandEvent) = 0;
};

}

#endif /* BERRYICOMMANDLISTENER_H_ */
