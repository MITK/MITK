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

#ifndef CHERRYWORKBENCHWINDOW_H_
#define CHERRYWORKBENCHWINDOW_H_

#include "cherryIWorkbenchWindow.h"

#include "internal/cherryWorkbenchWindowConfigurer.h"
#include "internal/cherryWindowManager.h"

#include "application/cherryWorkbenchAdvisor.h"
#include "application/cherryWorkbenchWindowAdvisor.h"
#include "application/cherryActionBarAdvisor.h"

#include <list>

namespace cherry {

struct IWorkbench;
struct IWorkbenchPage;
class  WorkbenchPage;
struct IPartService;
struct ISelectionService;
struct IPerspectiveDescriptor;

class Workbench;

/**
 * \ingroup org_opencherry_ui
 *
 */
class CHERRY_UI WorkbenchWindow : public IWorkbenchWindow
{
public:
  cherryClassMacro(WorkbenchWindow);

  WorkbenchWindow(int number);

  void Init();

  int Open();
  bool Close();

  SmartPointer<IWorkbenchPage> GetActivePage();

  /**
   * Sets the active page within the window.
   *
   * @param in
   *            identifies the new active page, or <code>null</code> for no
   *            active page
   */
  void SetActivePage(SmartPointer<IWorkbenchPage> in);

  IWorkbench* GetWorkbench();

  IPartService* GetPartService();

  ISelectionService* GetSelectionService();

  SmartPointer<IWorkbenchPage> OpenPage(const std::string& perspectiveId, IAdaptable* input);

  SmartPointer<IWorkbenchPage> OpenPage(IAdaptable* input);

  virtual int OpenImpl() = 0;
  virtual void* GetMenuManager() = 0;

  virtual bool SaveState(IMemento::Pointer memento);

  /**
   * Called when this window is about to be closed.
   *
   * Subclasses may overide to add code that returns <code>false</code> to
   * prevent closing under certain conditions.
   */
  virtual bool OkToClose();

  bool RestoreState(IMemento::Pointer memento,
      SmartPointer<IPerspectiveDescriptor> activeDescriptor);

  /**
   * Sets the window manager of this window.
   * <p>
   * Note that this method is used by <code>WindowManager</code> to maintain
   * a backpointer. Clients must not call the method directly.
   * </p>
   *
   * @param manager
   *            the window manager, or <code>null</code> if none
   */
  void SetWindowManager(WindowManager* manager);

  /**
   * Returns the window manager of this window.
   *
   * @return the WindowManager, or <code>null</code> if none
   */
  WindowManager* GetWindowManager();

  /**
   * Returns the number. This corresponds to a page number in a window or a
   * window number in the workbench.
   */
  int GetNumber();

protected:

  friend class WorkbenchConfigurer;
  friend class WorkbenchWindowConfigurer;
  friend class Workbench;

  /**
   * Creates the default contents and layout of the shell.
   *
   * @param shell
   *            the shell
   */
  virtual void CreateDefaultContents(void* shell) = 0;

  /**
   * Returns the GUI dependent page composite, under which the window's
   * pages create their controls.
   */
  virtual void* GetPageComposite() = 0;

  /**
   * Returns the unique object that applications use to configure this window.
   * <p>
   * IMPORTANT This method is declared package-private to prevent regular
   * plug-ins from downcasting IWorkbenchWindow to WorkbenchWindow and getting
   * hold of the workbench window configurer that would allow them to tamper
   * with the workbench window. The workbench window configurer is available
   * only to the application.
   * </p>
   */
  WorkbenchWindowConfigurer::Pointer GetWindowConfigurer();

  /**
   * Fills the window's real action bars.
   *
   * @param flags
   *            indicate which bars to fill
   */
  void FillActionBars(int flags);

  /**
   * The <code>WorkbenchWindow</code> implementation of this method
   * delegates to the window configurer.
   *
   * @since 3.0
   */
  Point GetInitialSize();

  /**
   * Returns the default page input for workbench pages opened in this window.
   *
   * @return the default page input or <code>null</code> if none
   * @since 3.1
   */
  IAdaptable* GetDefaultPageInput();

  bool IsClosing();

  /**
   * Opens a new page. Assumes that busy cursor is active.
   * <p>
   * <b>Note:</b> Since release 2.0, a window is limited to contain at most
   * one page. If a page exist in the window when this method is used, then
   * another window is created for the new page. Callers are strongly
   * recommended to use the <code>IWorkbench.openPerspective</code> APIs to
   * programmatically show a perspective.
   * </p>
   */
  SmartPointer<IWorkbenchPage> BusyOpenPage(
      const std::string& perspID, IAdaptable* input);


private:

  /**
   * Constant indicating that all the actions bars should be filled.
   *
   * @since 3.0
   */
  static const int FILL_ALL_ACTION_BARS;

  /**
   * Object for configuring this workbench window. Lazily initialized to an
   * instance unique to this window.
   *
   * @since 3.0
   */
  WorkbenchWindowConfigurer::Pointer windowConfigurer;

  WorkbenchWindowAdvisor* windowAdvisor;

  ActionBarAdvisor::Pointer actionBarAdvisor;

  WindowManager* windowManager;

  int number;
  bool closing;
  bool updateDisabled;


  class PageList {

  private:
    // List of pages in the order they were created;
    std::list<SmartPointer<IWorkbenchPage> > pagesInCreationOrder;

    // List of pages where the top is the last activated.
    std::list<SmartPointer<IWorkbenchPage> > pagesInActivationOrder;

    // The page explicitly activated
    SmartPointer<IWorkbenchPage> active;

  public:

    typedef std::list<SmartPointer<IWorkbenchPage> >::iterator iterator;

    bool Add(SmartPointer<IWorkbenchPage> object);

    iterator Begin();

    iterator End();

    void Clear();

    bool Contains(SmartPointer<IWorkbenchPage> object);

    bool Remove(SmartPointer<IWorkbenchPage> object);

    bool IsEmpty();

    const std::list<SmartPointer<IWorkbenchPage> >& GetPages();

    void SetActive(SmartPointer<IWorkbenchPage> page);

    SmartPointer<WorkbenchPage> GetActive();

    SmartPointer<WorkbenchPage> GetNextActive();
  };

  PageList pageList;


  /**
   * Notifies interested parties (namely the advisor) that the window is about
   * to be opened.
   *
   * @since 3.1
   */
  void FireWindowOpening();

  /**
   * Notifies interested parties (namely the advisor) that the window has been
   * restored from a previously saved state.
   *
   * @throws WorkbenchException
   *             passed through from the advisor
   * @since 3.1
   */
  void FireWindowRestored();

  /**
   * Notifies interested parties (namely the advisor) that the window has been
   * created.
   *
   * @since 3.1
   */
  void FireWindowCreated();

  /**
   * Notifies interested parties (namely the advisor and the window listeners)
   * that the window has been opened.
   *
   * @since 3.1
   */
  void FireWindowOpened();

  /**
   * Notifies interested parties (namely the advisor) that the window's shell
   * is closing. Allows the close to be vetoed.
   *
   * @return <code>true</code> if the close should proceed,
   *         <code>false</code> if it should be canceled
   * @since 3.1
   */
  bool FireWindowShellClosing();

  /**
   * Notifies interested parties (namely the advisor and the window listeners)
   * that the window has been closed.
   *
   * @since 3.1
   */
  void FireWindowClosed();

  /**
   * Returns the workbench advisor. Assumes the workbench has been created
   * already.
   * <p>
   * IMPORTANT This method is declared private to prevent regular plug-ins
   * from downcasting IWorkbenchWindow to WorkbenchWindow and getting hold of
   * the workbench advisor that would allow them to tamper with the workbench.
   * The workbench advisor is internal to the application.
   * </p>
   */
  /* private - DO NOT CHANGE */
  WorkbenchAdvisor* GetAdvisor();

  /**
   * Returns the window advisor, creating a new one for this window if needed.
   * <p>
   * IMPORTANT This method is declared private to prevent regular plug-ins
   * from downcasting IWorkbenchWindow to WorkbenchWindow and getting hold of
   * the window advisor that would allow them to tamper with the window. The
   * window advisor is internal to the application.
   * </p>
   */
  /* private - DO NOT CHANGE */
  WorkbenchWindowAdvisor* GetWindowAdvisor();

  /**
   * Returns the action bar advisor, creating a new one for this window if
   * needed.
   * <p>
   * IMPORTANT This method is declared private to prevent regular plug-ins
   * from downcasting IWorkbenchWindow to WorkbenchWindow and getting hold of
   * the action bar advisor that would allow them to tamper with the window's
   * action bars. The action bar advisor is internal to the application.
   * </p>
   */
  /* private - DO NOT CHANGE */
  ActionBarAdvisor::Pointer GetActionBarAdvisor();

  /*
   * Returns the IWorkbench implementation.
   */
  Workbench* GetWorkbenchImpl();

  /**
   * Close the window.
   *
   * Assumes that busy cursor is active.
   */
  bool BusyClose();

  /**
   * Unconditionally close this window. Assumes the proper flags have been set
   * correctly (e.i. closing and updateDisabled)
   */
  bool HardClose();

  /**
   * Close all of the pages.
   */
  void CloseAllPages();

  /**
   * Save all of the pages. Returns true if the operation succeeded.
   */
  bool SaveAllPages(bool bConfirm);

};

}

#endif /*CHERRYWORKBENCHWINDOW_H_*/
