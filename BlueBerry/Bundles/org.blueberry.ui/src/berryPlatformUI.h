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

#ifndef BERRYPLATFORMUI_H_
#define BERRYPLATFORMUI_H_

#include "berryUiDll.h"

#include "berryDisplay.h"
#include "application/berryWorkbenchAdvisor.h"
#include "testing/berryTestableObject.h"

namespace berry {

/**
 * \ingroup org_blueberry_ui
 *
 * The central class for access to the BlueBerry Platform User Interface.
 * This class cannot be instantiated; all functionality is provided by
 * static methods.
 *
 * Features provided:
 * <ul>
 * <li>creation of the workbench.</li>
 * <li>access to the workbench.</li>
 * </ul>
 * <p>
 *
 * @see IWorkbench
 */
class BERRY_UI PlatformUI
{
public:

  static const std::string PLUGIN_ID;
  static const std::string XP_WORKBENCH;
  static const std::string XP_VIEWS;

  /**
   * Return code (value 0) indicating that the workbench terminated normally.
   *
   * @see #CreateAndRunWorkbench
   * @since 3.0
   */
  static const int RETURN_OK;

  /**
   * Return code (value 1) indicating that the workbench was terminated with
   * a call to <code>IWorkbench.restart</code>.
   *
   * @see #CreateAndRunWorkbench
   * @see IWorkbench#Restart
   * @since 3.0
   */
  static const int RETURN_RESTART;

  /**
   * Return code (value 2) indicating that the workbench failed to start.
   *
   * @see #CreateAndRunWorkbench
   * @see IWorkbench#Restart
   * @since 3.0
   */
  static const int RETURN_UNSTARTABLE;

  /**
   * Return code (value 3) indicating that the workbench was terminated with
   * a call to IWorkbenchConfigurer#emergencyClose.
   *
   * @see #CreateAndRunWorkbench
   * @since 3.0
   */
  static const int RETURN_EMERGENCY_CLOSE;

  /**
   * Creates the workbench and associates it with the given display and workbench
   * advisor, and runs the workbench UI. This entails processing and dispatching
   * events until the workbench is closed or restarted.
   * <p>
   * This method is intended to be called by the main class (the "application").
   * Fails if the workbench UI has already been created.
   * </p>
   * <p>
   * Use {@link #createDisplay createDisplay} to create the display to pass in.
   * </p>
   * <p>
   * Note that this method is intended to be called by the application
   * (<code>org.blueberry.core.boot.IPlatformRunnable</code>). It must be
   * called exactly once, and early on before anyone else asks
   * <code>getWorkbench()</code> for the workbench.
   * </p>
   *
   * @param display the display to be used for all UI interactions with the workbench
   * @param advisor the application-specific advisor that configures and
   * specializes the workbench
   * @return return code {@link #RETURN_OK RETURN_OK} for normal exit;
   * {@link #RETURN_RESTART RETURN_RESTART} if the workbench was terminated
   * with a call to {@link IWorkbench#restart IWorkbench.restart};
   * {@link #RETURN_UNSTARTABLE RETURN_UNSTARTABLE} if the workbench could
   * not be started;
   * {@link #RETURN_EMERGENCY_CLOSE RETURN_EMERGENCY_CLOSE} if the UI quit
   * because of an emergency; other values reserved for future use
   */
  static int CreateAndRunWorkbench(Display* display, WorkbenchAdvisor* advisor);

  /**
   * Creates the <code>Display</code> to be used by the workbench.
   * It is the caller's responsibility to dispose the resulting <code>Display</code>,
   * not the workbench's.
   *
   * @return the display
   */
  static Display* CreateDisplay();

  /**
   * Returns the workbench. Fails if the workbench has not been created yet.
   *
   * @return the workbench. A raw pointer is returned because you normally
   * should not hold a smart pointer to it (and possibly create reference
   * cycles)
   */
  static IWorkbench* GetWorkbench();

  /**
   * Returns whether {@link #createAndRunWorkbench createAndRunWorkbench} has
   * been called to create the workbench, and the workbench has yet to
   * terminate.
   * <p>
   * Note that this method may return <code>true</code> while the workbench
   * is still being initialized, so it may not be safe to call workbench API
   * methods even if this method returns true. See bug 49316 for details.
   * </p>
   *
   * @return <code>true</code> if the workbench has been created and is
   *         still running, and <code>false</code> if the workbench has not
   *         yet been created or has completed
   * @since 3.0
   */
  static bool IsWorkbenchRunning();

  /**
   * Returns the testable object facade, for use by the test harness.
   * <p>
   * IMPORTANT: This method is only for use by the test harness.
   * Applications and regular plug-ins should not call this method.
   * </p>
   *
   * @return the testable object facade
   * @since 3.0
   */
  static TestableObject::Pointer GetTestableObject();

private:

  PlatformUI();

};

}

#endif /*BERRYPLATFORMUI_H_*/
