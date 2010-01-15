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


#ifndef CHERRYIWINDOWLISTENER_H_
#define CHERRYIWINDOWLISTENER_H_

#include "cherryUiDll.h"

#include <osgi/framework/Macros.h>
#include <osgi/framework/Message.h>

#include "cherryIWorkbenchWindow.h"

namespace cherry
{

/**
 * Interface for listening to window lifecycle events.
 * <p>
 * This interface may be implemented by clients.
 * </p>
 */
struct CHERRY_UI IWindowListener : public virtual Object {

  osgiInterfaceMacro(cherry::IWindowListener);

  struct Events {
    typedef Message1<IWorkbenchWindow::Pointer> WindowEvent;

    WindowEvent windowActivated;
    WindowEvent windowDeactivated;
    WindowEvent windowClosed;
    WindowEvent windowOpened;

    void AddListener(IWindowListener::Pointer listener);
    void RemoveListener(IWindowListener::Pointer listener);

  private:

    typedef MessageDelegate1<IWindowListener, IWorkbenchWindow::Pointer> Delegate;
  };

  /**
   * Notifies this listener that the given window has been activated.
   * <p>
   * <b>Note:</b> This event is not fired if no perspective is
   * open (the window is empty).
   * </p>
   *
   * @param window the window that was activated
   */
  virtual void WindowActivated(IWorkbenchWindow::Pointer /*window*/) {};

  /**
   * Notifies this listener that the given window has been deactivated.
   * <p>
   * <b>Note:</b> This event is not fired if no perspective is
   * open (the window is empty).
   * </p>
   *
   * @param window the window that was activated
   */
  virtual void WindowDeactivated(IWorkbenchWindow::Pointer /*window*/) {};

  /**
   * Notifies this listener that the given window has been closed.
   *
   * @param window the window that was closed
   * @see IWorkbenchWindow#close
   */
  virtual void WindowClosed(IWorkbenchWindow::Pointer /*window*/) {};

  /**
   * Notifies this listener that the given window has been opened.
   *
   * @param window the window that was opened
   * @see IWorkbench#openWorkbenchWindow
   */
  virtual void WindowOpened(IWorkbenchWindow::Pointer /*window*/) {};

};

}

#endif /* CHERRYIWINDOWLISTENER_H_ */
