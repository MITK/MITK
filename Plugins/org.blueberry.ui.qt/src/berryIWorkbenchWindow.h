/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIWORKBENCHWINDOW_H_
#define BERRYIWORKBENCHWINDOW_H_

#include <berryMacros.h>
#include <berryIAdaptable.h>

#include <org_blueberry_ui_qt_Export.h>

#include "berryIPageService.h"
#include "berryShell.h"

#include "services/berryIServiceLocator.h"

namespace berry {

struct IPartService;
struct ISelectionService;
struct IWorkbenchPage;
struct IWorkbench;

/**
 * \ingroup org_blueberry_ui_qt
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
struct BERRY_UI_QT IWorkbenchWindow : public IPageService, public IServiceLocator, public virtual Object
{

  berryObjectMacro(berry::IWorkbenchWindow, IPageService, IServiceLocator, Object);

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
   * Returns a list of the pages in this workbench window.
   * <p>
   * Note that each window has its own pages; pages are never shared between
   * different windows.
   * </p>
   *
   * @return a list of pages
   */
  virtual QList<SmartPointer<IWorkbenchPage> > GetPages() const = 0;

  /**
   * Returns the currently active page for this workbench window.
   *
   * @return the active page, or <code>null</code> if none
   */
  SmartPointer<IWorkbenchPage> GetActivePage() const override = 0;

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
  virtual ISelectionService* GetSelectionService() const = 0;

  /**
   * Returns this workbench window's shell.
   *
   * @return the shell containing this window's controls or <code>null</code>
   *   if the shell has not been created yet or if the window has been closed
   */
  virtual Shell::Pointer GetShell() const = 0;

  /**
   * Returns the workbench for this window.
   *
   * @return the workbench
   */
  virtual IWorkbench* GetWorkbench() const = 0;

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
  //virtual bool IsApplicationMenu(const QString& menuId) = 0;

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
  virtual SmartPointer<IWorkbenchPage> OpenPage(const QString& perspectiveId, IAdaptable* input) = 0;

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

  //virtual void SetPerspectiveExcludeList(const QStringList& v) = 0;
  //virtual QStringList GetPerspectiveExcludeList() const = 0;

  //virtual void SetViewExcludeList(const QStringList& v) = 0;
  //virtual QStringList GetViewExcludeList() const = 0;

  ~IWorkbenchWindow() override;

};

}

#endif /*BERRYIWORKBENCHWINDOW_H_*/
