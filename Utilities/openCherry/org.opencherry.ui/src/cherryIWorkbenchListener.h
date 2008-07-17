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


#ifndef CHERRYIWORKBENCHLISTENER_H_
#define CHERRYIWORKBENCHLISTENER_H_

#include "cherryUiDll.h"
#include "cherryMacros.h"

namespace cherry
{

struct IWorkbench;

/**
 * Interface for listening to workbench lifecycle events.
 * <p>
 * This interface may be implemented by clients.
 * </p>
 *
 * @see IWorkbench#addWorkbenchListener
 * @see IWorkbench#removeWorkbenchListener
 * @since 3.2
 */
struct CHERRY_UI IWorkbenchListener : public virtual Object {

  cherryInterfaceMacro(IWorkbenchListener, cherry);

  /**
   * Notifies that the workbench is about to shut down.
   * <p>
   * This method is called immediately prior to workbench shutdown before any
   * windows have been closed.
   * </p>
   * <p>
   * The listener may veto a regular shutdown by returning <code>false</code>,
   * although this will be ignored if the workbench is being forced to shut down.
   * </p>
   * <p>
   * Since other workbench listeners may veto the shutdown, the listener should
   * not dispose resources or do any other work during this notification that would
   * leave the workbench in an inconsistent state.
   * </p>
   *
   * @param workbench the workbench
   * @param forced <code>true</code> if the workbench is being forced to shutdown,
   *   <code>false</code> for a regular close
   * @return <code>true</code> to allow the workbench to proceed with shutdown,
   *   <code>false</code> to veto a non-forced shutdown
   */
  virtual bool PreShutdown(IWorkbench* workbench, bool forced) = 0;

  /**
   * Performs arbitrary finalization after the workbench stops running.
   * <p>
   * This method is called during workbench shutdown after all windows
   * have been closed.
   * </p>
   *
   * @param workbench the workbench
   */
  virtual void PostShutdown(IWorkbench* workbench);

};

}

#endif /* CHERRYIWORKBENCHLISTENER_H_ */
