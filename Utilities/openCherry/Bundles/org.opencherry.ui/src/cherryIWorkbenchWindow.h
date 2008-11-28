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

#ifndef CHERRYIWORKBENCHWINDOW_H_
#define CHERRYIWORKBENCHWINDOW_H_

#include <cherryMacros.h>
#include <cherryIAdaptable.h>

#include "cherryUiDll.h"

#include "cherryIPageService.h"
#include "cherryShell.h"

#include "services/cherryIServiceLocator.h"

namespace cherry {

struct IPartService;
struct ISelectionService;
struct IWorkbenchPage;
struct IWorkbench;

/**
 * \ingroup org_opencherry_ui
 *
 * A workbench window is a top level window in a workbench. Visually, a
 * workbench window has a menubar, a toolbar, a status bar, and a main area for
 * displaying a single page consisting of a collection of views and editors.
 * <p>
 * Each workbench window has a collection of 0 or more pages; the active page is
 * the one that is being presented to the end user; at most one page is active
 * in a window at a time.
 * </p>
 * <p>
 * The workbench window supports a few {@link IServiceLocator services} by
 * default. If these services are used to allocate resources, it is important to
 * remember to clean up those resources after you are done with them. Otherwise,
 * the resources will exist until the workbench window is closed. The supported
 * services are:
 * </p>
 * <ul>
 * <li>{@link ICommandService}</li>
 * <li>{@link IContextService}</li>
 * <li>{@link IHandlerService}</li>
 * <li>{@link IBindingService}. Resources allocated through this service will
 * not be cleaned up until the workbench shuts down.</li>
 * </ul>
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 *
 * @see IWorkbenchPage
 * @noimplement This interface is not intended to be implemented by clients.
 *
 */
struct CHERRY_UI IWorkbenchWindow : public IPageService, public IServiceLocator, public virtual Object
{

  cherryClassMacro(IWorkbenchWindow);

  /**
   * Closes this workbench window.
   * <p>
   * If the window has an open editor with unsaved content, the user will be
   * given the opportunity to save it.
   * </p>
   *
   * @return <code>true</code> if the window was successfully closed, and
   *         <code>false</code> if it is still open
   */
  virtual bool Close() = 0;

  /**
   * Returns the currently active page for this workbench window.
   *
   * @return the active page, or <code>null</code> if none
   */
  virtual SmartPointer<IWorkbenchPage> GetActivePage() = 0;

  /**
   * Sets or clears the currently active page for this workbench window.
   *
   * @param page
   *            the new active page
   */
  virtual void SetActivePage(SmartPointer<IWorkbenchPage> page) = 0;

  /**
   * Returns the part service which tracks part activation within this
   * workbench window.
   *
   * @return the part service
   */
  virtual IPartService* GetPartService() = 0;

  /**
   * Returns the selection service which tracks selection within this
   * workbench window.
   *
   * @return the selection service
   */
  virtual ISelectionService* GetSelectionService() = 0;

  /**
   * Returns this workbench window's shell.
   *
   * @return the shell containing this window's controls or <code>null</code>
   *   if the shell has not been created yet or if the window has been closed
   */
  virtual Shell::Pointer GetShell() = 0;

  /**
   * Returns the workbench for this window.
   *
   * @return the workbench
   */
  virtual IWorkbench* GetWorkbench() = 0;

  /**
   * Returns whether the specified menu is an application menu as opposed to
   * a part menu. Application menus contain items which affect the workbench
   * or window. Part menus contain items which affect the active part (view
   * or editor).
   * <p>
   * This is typically used during "in place" editing. Application menus
   * should be preserved during menu merging. All other menus may be removed
   * from the window.
   * </p>
   *
   * @param menuId
   *            the menu id
   * @return <code>true</code> if the specified menu is an application
   *         menu, and <code>false</code> if it is not
   */
  //virtual bool IsApplicationMenu(const std::string& menuId) = 0;

  /**
   * Creates and opens a new workbench page. The perspective of the new page
   * is defined by the specified perspective ID. The new page become active.
   * <p>
   * <b>Note:</b> Since release 2.0, a window is limited to contain at most
   * one page. If a page exist in the window when this method is used, then
   * another window is created for the new page. Callers are strongly
   * recommended to use the <code>IWorkbench.showPerspective</code> APIs to
   * programmatically show a perspective.
   * </p>
   *
   * @param perspectiveId
   *            the perspective id for the window's initial page
   * @param input
   *            the page input, or <code>null</code> if there is no current
   *            input. This is used to seed the input for the new page's
   *            views.
   * @return the new workbench page
   * @exception WorkbenchException
   *                if a page could not be opened
   *
   * @see IWorkbench#showPerspective(String, IWorkbenchWindow, IAdaptable)
   */
  virtual SmartPointer<IWorkbenchPage> OpenPage(const std::string& perspectiveId, IAdaptable* input) = 0;

  /**
   * Creates and opens a new workbench page. The default perspective is used
   * as a template for creating the page. The page becomes active.
   * <p>
   * <b>Note:</b> Since release 2.0, a window is limited to contain at most
   * one page. If a page exist in the window when this method is used, then
   * another window is created for the new page. Callers are strongly
   * recommended to use the <code>IWorkbench.showPerspective</code> APIs to
   * programmatically show a perspective.
   * </p>
   *
   * @param input
   *            the page input, or <code>null</code> if there is no current
   *            input. This is used to seed the input for the new page's
   *            views.
   * @return the new workbench window
   * @exception WorkbenchException
   *                if a page could not be opened
   *
   * @see IWorkbench#showPerspective(String, IWorkbenchWindow, IAdaptable)
   */
  virtual SmartPointer<IWorkbenchPage> OpenPage(IAdaptable* input) = 0;


  virtual ~IWorkbenchWindow() {}

};

}

#endif /*CHERRYIWORKBENCHWINDOW_H_*/
