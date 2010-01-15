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

#ifndef CHERRYIWORKBENCH_H_
#define CHERRYIWORKBENCH_H_

#include <cherryMacros.h>

#include "services/cherryIServiceLocator.h"
#include "dialogs/cherryIDialog.h"
#include "cherryIViewRegistry.h"
#include "cherryIEditorRegistry.h"
#include "intro/cherryIIntroManager.h"
#include "cherryIPerspectiveRegistry.h"
#include "cherryIWorkbenchWindow.h"
#include "cherryIWorkbenchListener.h"
#include "cherryIWindowListener.h"
#include "cherryDisplay.h"

namespace cherry {

struct IWorkbenchPage;

/**
 * \ingroup org_opencherry_ui
 *
 * A workbench is the root object for the openCherry Platform user interface.
 * <p>
 * A <b>workbench</b> has one or more main windows which present to the end
 * user information based on some underlying model, typically on resources in an
 * underlying workspace. A workbench usually starts with a single open window,
 * and automatically closes when its last window closes.
 * </p>
 * <p>
 * Each <b>workbench window</b> has a collection of <b>pages</b>; the active
 * page is the one that is being presented to the end user; at most one page is
 * active in a window at a time.
 * </p>
 * <p>
 * Each workbench page has a collection of <b>workbench parts</b>, of which
 * there are two kinds: views and editors. A page's parts are arranged (tiled or
 * stacked) for presentation on the screen. The arrangement is not fixed; the
 * user can arrange the parts as they see fit. A <b>perspective</b> is a
 * template for a page, capturing a collection of parts and their arrangement.
 * </p>
 * <p>
 * The platform creates a workbench when the workbench plug-in is activated;
 * since this happens at most once during the life of the running platform,
 * there is only one workbench instance. Due to its singular nature, it is
 * commonly referred to as <it>the</it> workbench.
 * </p>
 * <p>
 * The workbench supports a few {@link IServiceLocator services} by default. If
 * these services are used to allocate resources, it is important to remember to
 * clean up those resources after you are done with them. Otherwise, the
 * resources will exist until the workbench shuts down. The supported services
 * are:
 * </p>
 * <ul>
 * <li>{@link IBindingService}</li>
 * <li>{@link ICommandService}</li>
 * <li>{@link IContextService}</li>
 * <li>{@link IHandlerService}</li>
 * </ul>
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 *
 * @see PlatformUI#getWorkbench
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct CHERRY_UI IWorkbench : public IServiceLocator {

  cherryInterfaceMacro(IWorkbench, cherry)

  virtual ~IWorkbench() {}

  /**
   * Returns the display for this workbench.
   * <p>
   * Code should always ask the workbench for the display rather than rely on
   * {@link Display#getDefault Display.getDefault()}.
   * </p>
   *
   * @return the display to be used for all UI interactions with this
   *         workbench
   */
  virtual Display* GetDisplay() = 0;

  /**
   * Adds a workbench listener.
   *
   * @param listener
   *            the workbench listener to add
   * @since 3.2
   */
  virtual void AddWorkbenchListener(IWorkbenchListener::Pointer listener) = 0;

  /**
   * Removes a workbench listener.
   *
   * @param listener
   *            the workbench listener to remove
   * @since 3.2
   */
  virtual void RemoveWorkbenchListener(IWorkbenchListener::Pointer listener) = 0;

  /**
   * Returns the workbench events object
   */
  virtual IWorkbenchListener::Events& GetWorkbenchEvents() = 0;

  /**
   * Adds a window listener.
   *
   * @param listener
   *            the window listener to add
   * @since 2.0
   */
  virtual void AddWindowListener(IWindowListener::Pointer listener) = 0;

  /**
   * Removes a window listener.
   *
   * @param listener
   *            the window listener to remove
   * @since 2.0
   */
  virtual void RemoveWindowListener(IWindowListener::Pointer listener) = 0;

  /**
   * Returns the window events object
   *
   */
  virtual IWindowListener::Events& GetWindowEvents() = 0;

  /**
   * Closes this workbench and all its open windows.
   * <p>
   * If the workbench has an open editor with unsaved content, the user will
   * be given the opportunity to save it.
   * </p>
   *
   * @return <code>true</code> if the workbench was successfully closed, and
   *         <code>false</code> if it is still open
   */
  virtual bool Close() = 0;

  /**
   * Returns the currently active window for this workbench (if any). Returns
   * <code>null</code> if there is no active workbench window. Returns
   * <code>null</code> if called from a non-UI thread.
   *
   * @return the active workbench window, or <code>null</code> if there is
   *         no active workbench window or if called from a non-UI thread
   */
  virtual IWorkbenchWindow::Pointer GetActiveWorkbenchWindow() = 0;

  /**
   * Returns the perspective registry for the workbench.
   *
   * @return the workbench perspective registry
   */
  virtual IPerspectiveRegistry* GetPerspectiveRegistry() = 0;

  /**
   * Returns the view registry for the workbench.
   *
   * @return the workbench view registry
   * @since 3.1
   */
  virtual IViewRegistry* GetViewRegistry() = 0;

  /**
   * Returns the editor registry for the workbench.
   *
   * @return the workbench editor registry
   */
  virtual IEditorRegistry* GetEditorRegistry() = 0;

  /**
   * Returns the number of open main windows associated with this workbench.
   * Note that wizards and dialogs are not included in this list since they
   * are not considered main windows.
   *
   * @return the number of open windows
   * @since 3.0
   */
  virtual std::size_t GetWorkbenchWindowCount() = 0;

  /**
   * Returns a list of the open main windows associated with this workbench.
   * Note that wizards and dialogs are not included in this list since they
   * are not considered main windows.
   *
   * @return a list of open windows
   */
  virtual std::vector<IWorkbenchWindow::Pointer> GetWorkbenchWindows() = 0;

  /**
   * Creates and opens a new workbench window with one page. The perspective
   * of the new page is defined by the specified perspective ID. The new
   * window and new page become active.
   * <p>
   * <b>Note:</b> The caller is responsible to ensure the action using this
   * method will explicitly inform the user a new window will be opened.
   * Otherwise, callers are strongly recommended to use the
   * <code>openPerspective</code> APIs to programmatically show a
   * perspective to avoid confusing the user.
   * </p>
   * <p>
   * In most cases where this method is used the caller is tightly coupled to
   * a particular perspective. They define it in the registry and contribute
   * some user interface action to open or activate it. In situations like
   * this a static variable is often used to identify the perspective ID.
   * </p>
   *
   * @param perspectiveId
   *            the perspective id for the window's initial page, or
   *            <code>null</code> for no initial page
   * @param input
   *            the page input, or <code>null</code> if there is no current
   *            input. This is used to seed the input for the new page's
   *            views.
   * @return the new workbench window
   * @exception WorkbenchException
   *                if a new window and page could not be opened
   *
   * @see IWorkbench#showPerspective(String, IWorkbenchWindow, IAdaptable)
   */
  virtual IWorkbenchWindow::Pointer OpenWorkbenchWindow(const std::string& perspectiveId,
      IAdaptable* input) = 0;

  /**
   * Creates and opens a new workbench window with one page. The perspective
   * of the new page is defined by the default perspective ID. The new window
   * and new page become active.
   * <p>
   * <b>Note:</b> The caller is responsible to ensure the action using this
   * method will explicitly inform the user a new window will be opened.
   * Otherwise, callers are strongly recommended to use the
   * <code>openPerspective</code> APIs to programmatically show a
   * perspective to avoid confusing the user.
   * </p>
   *
   * @param input
   *            the page input, or <code>null</code> if there is no current
   *            input. This is used to seed the input for the new page's
   *            views.
   * @return the new workbench window
   * @exception WorkbenchException
   *                if a new window and page could not be opened
   *
   * @see IWorkbench#showPerspective(String, IWorkbenchWindow, IAdaptable)
   */
  virtual IWorkbenchWindow::Pointer OpenWorkbenchWindow(IAdaptable* input) = 0;

  /**
   * Shows the specified perspective to the user. The caller should use this
   * method when the perspective to be shown is not dependent on the page's
   * input. That is, the perspective can open in any page depending on user
   * preferences.
   * <p>
   * The perspective may be shown in the specified window, in another existing
   * window, or in a new window depending on user preferences. The exact
   * policy is controlled by the workbench to ensure consistency to the user.
   * The policy is subject to change. The current policy is as follows:
   * <ul>
   * <li>If the specified window has the requested perspective open, then the
   * window is given focus and the perspective is shown. The page's input is
   * ignored.</li>
   * <li>If another window that has the workspace root as input and the
   * requested perspective open and active, then the window is given focus.
   * </li>
   * <li>Otherwise the requested perspective is opened and shown in the
   * specified window or in a new window depending on the current user
   * preference for opening perspectives, and that window is given focus.
   * </li>
   * </ul>
   * </p>
   * <p>
   * The workbench also defines a number of menu items to activate or open
   * each registered perspective. A complete list of these perspectives is
   * available from the perspective registry found on <code>IWorkbench</code>.
   * </p>
   *
   * @param perspectiveId
   *            the perspective ID to show
   * @param window
   *            the workbench window of the action calling this method.
   * @return the workbench page that the perspective was shown
   * @exception WorkbenchException
   *                if the perspective could not be shown
   *
   * @since 2.0
   */
  virtual SmartPointer<IWorkbenchPage> ShowPerspective(const std::string& perspectiveId,
      IWorkbenchWindow::Pointer window) = 0;

  /**
   * Shows the specified perspective to the user. The caller should use this
   * method when the perspective to be shown is dependent on the page's input.
   * That is, the perspective can only open in any page with the specified
   * input.
   * <p>
   * The perspective may be shown in the specified window, in another existing
   * window, or in a new window depending on user preferences. The exact
   * policy is controlled by the workbench to ensure consistency to the user.
   * The policy is subject to change. The current policy is as follows:
   * <ul>
   * <li>If the specified window has the requested perspective open and the
   * same requested input, then the window is given focus and the perspective
   * is shown.</li>
   * <li>If another window has the requested input and the requested
   * perspective open and active, then that window is given focus.</li>
   * <li>If the specified window has the same requested input but not the
   * requested perspective, then the window is given focus and the perspective
   * is opened and shown on condition that the user preference is not to open
   * perspectives in a new window.</li>
   * <li>Otherwise the requested perspective is opened and shown in a new
   * window, and the window is given focus.</li>
   * </ul>
   * </p>
   * <p>
   * The workbench also defines a number of menu items to activate or open
   * each registered perspective. A complete list of these perspectives is
   * available from the perspective registry found on <code>IWorkbench</code>.
   * </p>
   *
   * @param perspectiveId
   *            the perspective ID to show
   * @param window
   *            the workbench window of the action calling this method.
   * @param input
   *            the page input, or <code>null</code> if there is no current
   *            input. This is used to seed the input for the page's views
   * @return the workbench page that the perspective was shown
   * @exception WorkbenchException
   *                if the perspective could not be shown
   *
   * @since 2.0
   */
  virtual SmartPointer<IWorkbenchPage> ShowPerspective(const std::string& perspectiveId,
      IWorkbenchWindow::Pointer window, IAdaptable* input) = 0;

  /**
   * Save all dirty editors in the workbench. Opens a dialog to prompt the
   * user if <code>confirm</code> is true. Return true if successful. Return
   * false if the user has canceled the command.
   *
   * @param confirm <code>true</code> to ask the user before saving unsaved
   *            changes (recommended), and <code>false</code> to save
   *            unsaved changes without asking
   * @return <code>true</code> if the command succeeded, and
   *         <code>false</code> if the operation was canceled by the user or
   *         an error occurred while saving
   */
  virtual bool SaveAllEditors(bool confirm) = 0;

  /**
   * Return the intro manager for this workbench.
   *
   * @return the intro manager for this workbench. Guaranteed not to be
   *         <code>null</code>.
   */
  virtual IIntroManager* GetIntroManager() = 0;

  /**
   * Returns a boolean indicating whether the workbench is in the process of
   * closing.
   *
   * @return <code>true</code> if the workbench is in the process of
   *         closing, <code>false</code> otherwise
   * @since 3.1
   */
  virtual bool IsClosing() = 0;

  /**
   * Applies changes of the current theme to the user interface.
   */
  virtual void UpdateTheme() = 0;

};

}

#endif /*CHERRYIWORKBENCH_H_*/
