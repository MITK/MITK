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


#ifndef BERRYIWINDOWLISTENER_H_
#define BERRYIWINDOWLISTENER_H_

#include <org_blueberry_ui_qt_Export.h>

#include <berryMessage.h>

#include "berryIWorkbenchWindow.h"

namespace berry
{

/**
 * Interface for listening to window lifecycle events.
 * <p>
 * This interface may be implemented by clients.
 * </p>
 */
struct BERRY_UI_QT IWindowListener
{

  struct Events {
    typedef Message1<const IWorkbenchWindow::Pointer&> WindowEvent;

    WindowEvent windowActivated;
    WindowEvent windowDeactivated;
    WindowEvent windowClosed;
    WindowEvent windowOpened;

    void AddListener(IWindowListener* listener);
    void RemoveListener(IWindowListener* listener);

  private:

    typedef MessageDelegate1<IWindowListener, const IWorkbenchWindow::Pointer&> Delegate;
  };

  virtual ~IWindowListener();

  /**
   * Notifies this listener that the given window has been activated.
   * <p>
   * <b>Note:</b> This event is not fired if no perspective is
   * open (the window is empty).
   * </p>
   *
   * @param window the window that was activated
   */
  virtual void WindowActivated(const IWorkbenchWindow::Pointer& /*window*/) {}

  /**
   * Notifies this listener that the given window has been deactivated.
   * <p>
   * <b>Note:</b> This event is not fired if no perspective is
   * open (the window is empty).
   * </p>
   *
   * @param window the window that was activated
   */
  virtual void WindowDeactivated(const IWorkbenchWindow::Pointer& /*window*/) {}

  /**
   * Notifies this listener that the given window has been closed.
   *
   * @param window the window that was closed
   * @see IWorkbenchWindow#close
   */
  virtual void WindowClosed(const IWorkbenchWindow::Pointer& /*window*/) {}

  /**
   * Notifies this listener that the given window has been opened.
   *
   * @param window the window that was opened
   * @see IWorkbench#openWorkbenchWindow
   */
  virtual void WindowOpened(const IWorkbenchWindow::Pointer& /*window*/) {}

};

}

#endif /* BERRYIWINDOWLISTENER_H_ */
