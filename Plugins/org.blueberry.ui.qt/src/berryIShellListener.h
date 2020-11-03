/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYISHELLLISTENER_H_
#define BERRYISHELLLISTENER_H_

#include <org_blueberry_ui_qt_Export.h>

#include "berryShellEvent.h"

#include <berryMessage.h>
#include <berryMacros.h>

namespace berry {

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
struct BERRY_UI_QT IShellListener
{

  struct Events {
    typedef Message1<const ShellEvent::Pointer&> ShellEventType;

    enum Type {
     NONE        = 0x00000000,
     ACTIVATED   = 0x00000001,
     CLOSED      = 0x00000002,
     DEACTIVATED = 0x00000004,
     DEICONIFIED = 0x00000008,
     ICONIFIED   = 0x00000010,

     ALL         = 0xffffffff
    };

    Q_DECLARE_FLAGS(Types, Type)

    ShellEventType shellActivated;
    ShellEventType shellClosed;
    ShellEventType shellDeactivated;
    ShellEventType shellDeiconified;
    ShellEventType shellIconified;

    void AddListener(IShellListener* listener);
    void RemoveListener(IShellListener* listener);

  private:

    typedef MessageDelegate1<IShellListener, const ShellEvent::Pointer&> Delegate;
  };

  virtual ~IShellListener();

  /**
   * Sent when a shell becomes the active window.
   *
   * @param e an event containing information about the activation
   */
  virtual void ShellActivated(const ShellEvent::Pointer& e);

  /**
   * Sent when a shell is closed.
   *
   * @param e an event containing information about the close
   */
  virtual void ShellClosed(const ShellEvent::Pointer& e);

  /**
   * Sent when a shell stops being the active window.
   *
   * @param e an event containing information about the deactivation
   */
  virtual void ShellDeactivated(const ShellEvent::Pointer& e);

  /**
   * Sent when a shell is un-minimized.
   *
   * @param e an event containing information about the un-minimization
   */
  virtual void ShellDeiconified(const ShellEvent::Pointer& e);

  /**
   * Sent when a shell is minimized.
   *
   * @param e an event containing information about the minimization
   */
  virtual void ShellIconified(const ShellEvent::Pointer& e);
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(berry::IShellListener::Events::Types)

#endif /* BERRYISHELLLISTENER_H_ */
