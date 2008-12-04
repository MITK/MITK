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


#ifndef CHERRYISHELLLISTENER_H_
#define CHERRYISHELLLISTENER_H_

#include "cherryUiDll.h"

#include "cherryShellEvent.h"

#include <cherryMessage.h>
#include <cherryMacros.h>

namespace cherry {

/**
 * Classes which implement this interface provide methods
 * that deal with changes in state of <code>Shell</code>s.
 * <p>
 * After creating an instance of a class that implements
 * this interface it can be added to a shell using the
 * <code>AddShellListener</code> method and removed using
 * the <code>RemoveShellListener</code> method. When the
 * state of the shell changes, the appropriate method will
 * be invoked.
 * </p>
 *
 * @see ShellEvent
 */
struct CHERRY_UI IShellListener : public virtual Object {

  cherryClassMacro(IShellListener);

  struct Events {
    typedef Message1<ShellEvent::Pointer> ShellEventType;

    enum Type {
     NONE        = 0x00000000,
     ACTIVATED   = 0x00000001,
     CLOSED      = 0x00000002,
     DEACTIVATED = 0x00000004,
     DEICONIFIED = 0x00000008,
     ICONIFIED   = 0x00000010,

     ALL         = 0xffffffff
    };

    CHERRY_DECLARE_FLAGS(Types, Type)

    ShellEventType shellActivated;
    ShellEventType shellClosed;
    ShellEventType shellDeactivated;
    ShellEventType shellDeiconified;
    ShellEventType shellIconified;

    void AddListener(IShellListener::Pointer listener);
    void RemoveListener(IShellListener::Pointer listener);

  private:

    typedef MessageDelegate1<IShellListener, ShellEvent::Pointer> Delegate;
  };

/**
 * Sent when a shell becomes the active window.
 *
 * @param e an event containing information about the activation
 */
virtual void ShellActivated(ShellEvent::Pointer e);

/**
 * Sent when a shell is closed.
 *
 * @param e an event containing information about the close
 */
virtual void ShellClosed(ShellEvent::Pointer e);

/**
 * Sent when a shell stops being the active window.
 *
 * @param e an event containing information about the deactivation
 */
virtual void ShellDeactivated(ShellEvent::Pointer e);

/**
 * Sent when a shell is un-minimized.
 *
 * @param e an event containing information about the un-minimization
 */
virtual void ShellDeiconified(ShellEvent::Pointer e);

/**
 * Sent when a shell is minimized.
 *
 * @param e an event containing information about the minimization
 */
virtual void ShellIconified(ShellEvent::Pointer e);
};

}

CHERRY_DECLARE_OPERATORS_FOR_FLAGS(cherry::IShellListener::Events::Types)

#endif /* CHERRYISHELLLISTENER_H_ */
