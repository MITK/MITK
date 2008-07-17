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

namespace cherry
{

/**
 * Interface for listening to window lifecycle events.
 * <p>
 * This interface may be implemented by clients.
 * </p>
 */
struct CHERRY_UI IWindowListener : public virtual Object {

  cherryInterfaceMacro(IWindowListener, cherry);

  virtual ~IWindowListener() {}

  /**
   * Notifies this listener that the given window has been activated.
   * <p>
   * <b>Note:</b> This event is not fired if no perspective is
   * open (the window is empty).
   * </p>
   *
   * @param window the window that was activated
   */
  virtual void WindowActivated(IWorkbenchWindow::Pointer window) = 0;

  /**
   * Notifies this listener that the given window has been deactivated.
   * <p>
   * <b>Note:</b> This event is not fired if no perspective is
   * open (the window is empty).
   * </p>
   *
   * @param window the window that was activated
   */
  virtual void WindowDeactivated(IWorkbenchWindow::Pointer window) = 0;

  /**
   * Notifies this listener that the given window has been closed.
   *
   * @param window the window that was closed
   * @see IWorkbenchWindow#close
   */
  virtual void WindowClosed(IWorkbenchWindow::Pointer window) = 0;

  /**
   * Notifies this listener that the given window has been opened.
   *
   * @param window the window that was opened
   * @see IWorkbench#openWorkbenchWindow
   */
  virtual void WindowOpened(IWorkbenchWindow::Pointer window) = 0;

};

}

#endif /* CHERRYIWINDOWLISTENER_H_ */
