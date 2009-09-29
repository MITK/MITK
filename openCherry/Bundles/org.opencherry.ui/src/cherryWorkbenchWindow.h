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

#include "cherryIPerspectiveListener.h"
#include "guitk/cherryGuiTkIControlListener.h"
#include "cherryWindow.h"

#include "internal/cherryWorkbenchWindowConfigurer.h"
#include "internal/cherryShellPool.h"
#include "internal/cherryServiceLocator.h"
#include "internal/cherryWWinPartService.h"

#include "application/cherryWorkbenchAdvisor.h"
#include "application/cherryWorkbenchWindowAdvisor.h"
#include "application/cherryActionBarAdvisor.h"

#include <list>

namespace cherry
{

struct IWorkbench;
struct IWorkbenchPage;
class WorkbenchPage;
struct IPartService;
struct ISelectionService;
struct IPerspectiveDescriptor;

class Workbench;

/**
 * \ingroup org_opencherry_ui
 *
 */
class WorkbenchWindow: public Window, public IWorkbenchWindow
{
public:
  cherryObjectMacro(WorkbenchWindow)
  ;

  WorkbenchWindow(int number);

  ~WorkbenchWindow();

  Object::Pointer GetService(const std::string& key);

  bool HasService(const std::string& key) const;

  int Open();
  bool Close();

  Shell::Pointer GetShell();

  /**
   * @see org.opencherry.ui.IPageService
   */
  void AddPerspectiveListener(IPerspectiveListener::Pointer l);

  /**
   * @see org.opencherry.ui.IPageService
   */
  void RemovePerspectiveListener(IPerspectiveListener::Pointer l);

  /**
   * @see org.opencherry.ui.IPageService
   */
  IPerspectiveListener::Events& GetPerspectiveEvents();

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

  SmartPointer<IWorkbenchPage> OpenPage(const std::string& perspectiveId,
      IAdaptable* input);

  SmartPointer<IWorkbenchPage> OpenPage(IAdaptable* input);

  //TODO menu manager
  //virtual void* GetMenuManager() = 0;

  virtual bool SaveState(IMemento::Pointer memento);

  /**
   * Called when this window is about to be closed.
   *
   * Subclasses may overide to add code that returns <code>false</code> to
   * prevent closing under certain conditions.
   */
  virtual bool OkToClose();

  bool RestoreState(IMemento::Pointer memento, SmartPointer<
      IPerspectiveDescriptor> activeDescriptor);

  /**
   * Returns the number. This corresponds to a page number in a window or a
   * window number in the workbench.
   */
  int GetNumber();

  /**
   * <p>
   * Indicates the start of a large update within this window. This is used to
   * disable CPU-intensive, change-sensitive services that were temporarily
   * disabled in the midst of large changes. This method should always be
   * called in tandem with <code>largeUpdateEnd</code>, and the event loop
   * should not be allowed to spin before that method is called.
   * </p>
   * <p>
   * Important: always use with <code>largeUpdateEnd</code>!
   * </p>
   *
   * @since 3.1
   */
  void LargeUpdateStart();

  /**
   * <p>
   * Indicates the end of a large update within this window. This is used to
   * re-enable services that were temporarily disabled in the midst of large
   * changes. This method should always be called in tandem with
   * <code>largeUpdateStart</code>, and the event loop should not be
   * allowed to spin before this method is called.
   * </p>
   * <p>
   * Important: always protect this call by using <code>finally</code>!
   * </p>
   *
   * @since 3.1
   */
  void LargeUpdateEnd();

protected:

  friend class WorkbenchConfigurer;
  friend class WorkbenchWindowConfigurer;
  friend class WorkbenchWindowConfigurer::WindowActionBarConfigurer;
  friend class Workbench;
  friend class LayoutPartSash;
  friend class WorkbenchPage;
  friend class DetachedWindow;

  /**
   * Returns the GUI dependent page composite, under which the window's
   * pages create their controls.
   */
  void* GetPageComposite();

  /**
   * Creates and remembers the client composite, under which workbench pages
   * create their controls.
   *
   * @since 3.0
   */
  void* CreatePageComposite(void* parent);

  /**
   * Creates the contents of the workbench window, including trim controls and
   * the client composite. This MUST create the client composite via a call to
   * <code>createClientComposite</code>.
   *
   * @since 3.0
   */
  void* CreateContents(Shell::Pointer parent);

  /**
   * Creates the default contents and layout of the shell.
   *
   * @param shell
   *            the shell
   */
  void CreateDefaultContents(Shell::Pointer shell);

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

  /*
   * @see cherry::Window#configureShell(Shell::Pointer)
   */
  void ConfigureShell(Shell::Pointer shell);

  ShellPool::Pointer GetDetachedWindowPool();

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
  SmartPointer<IWorkbenchPage> BusyOpenPage(const std::string& perspID,
      IAdaptable* input);

  bool ClosePage(SmartPointer<IWorkbenchPage> in, bool save);

private:

  /**
   * Constant indicating that all the actions bars should be filled.
   *
   * @since 3.0
   */
  static const int FILL_ALL_ACTION_BARS;

  /**
   * The composite under which workbench pages create their controls.
   *
   * @since 3.0
   */
  void* pageComposite;

  ShellPool::Pointer detachedWindowShells;

  /**
   * Object for configuring this workbench window. Lazily initialized to an
   * instance unique to this window.
   *
   * @since 3.0
   */
  WorkbenchWindowConfigurer::Pointer windowConfigurer;

  WorkbenchWindowAdvisor* windowAdvisor;

  ActionBarAdvisor::Pointer actionBarAdvisor;

  int number;

  /**
   * The number of large updates that are currently going on. If this is
   * number is greater than zero, then UI updateActionBars is a no-op.
   */
  int largeUpdates;

  bool closing;
  bool shellActivated;
  bool updateDisabled;

  /**
   * The map of services maintained by the workbench window. These services
   * are initialized during workbench window during the
   * {@link #configureShell(Shell)}.
   */
  ServiceLocator::Pointer serviceLocator;

  bool emptyWindowContentsCreated;
  void* emptyWindowContents;

  Rectangle normalBounds;

  bool asMaximizedState;

  IPerspectiveListener::Events perspectiveEvents;

  WWinPartService partService;

  struct ServiceLocatorOwner: public IDisposable
  {
    ServiceLocatorOwner(WorkbenchWindow* wnd);

    WorkbenchWindow* window;

    void Dispose();
  };

  IDisposable::Pointer serviceLocatorOwner;

  class PageList
  {

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

  //  /**
  //   * Fires page activated
  //   */
  //  void FirePageActivated(IWorkbenchPage::Pointer page);
  //
  //  /**
  //   * Fires page closed
  //   */
  //  void FirePageClosed(IWorkbenchPage::Pointer page);
  //
  //  /**
  //   * Fires page opened
  //   */
  //  void FirePageOpened(IWorkbenchPage::Pointer page);

  /**
   * Fires perspective activated
   */
  void FirePerspectiveActivated(SmartPointer<IWorkbenchPage> page,
      IPerspectiveDescriptor::Pointer perspective);

  /**
   * Fires perspective deactivated.
   *
   * @since 3.2
   */
  void FirePerspectivePreDeactivate(SmartPointer<IWorkbenchPage> page,
      IPerspectiveDescriptor::Pointer perspective);

  /**
   * Fires perspective deactivated.
   *
   * @since 3.1
   */
  void FirePerspectiveDeactivated(SmartPointer<IWorkbenchPage> page,
      IPerspectiveDescriptor::Pointer perspective);

  /**
   * Fires perspective changed
   */
  void FirePerspectiveChanged(SmartPointer<IWorkbenchPage> ,
      IPerspectiveDescriptor::Pointer perspective, const std::string& changeId);

  /**
   * Fires perspective changed for an affected part
   */
  void FirePerspectiveChanged(SmartPointer<IWorkbenchPage> ,
      IPerspectiveDescriptor::Pointer perspective,
      IWorkbenchPartReference::Pointer partRef, const std::string& changeId);

  /**
   * Fires perspective closed
   */
  void FirePerspectiveClosed(SmartPointer<IWorkbenchPage> ,
      IPerspectiveDescriptor::Pointer perspective);

  /**
   * Fires perspective opened
   */
  void FirePerspectiveOpened(SmartPointer<IWorkbenchPage> ,
      IPerspectiveDescriptor::Pointer perspective);

  /**
   * Fires perspective saved as.
   *
   * @since 3.1
   */
  void FirePerspectiveSavedAs(SmartPointer<IWorkbenchPage> ,
      IPerspectiveDescriptor::Pointer oldPerspective,
      IPerspectiveDescriptor::Pointer newPerspective);

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

  bool UnableToRestorePage(IMemento::Pointer pageMem);

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

  void ShowEmptyWindowContents();

  void HideEmptyWindowContents();

  struct ShellActivationListener: public IShellListener
  {
    ShellActivationListener(WorkbenchWindow::Pointer window);

    void ShellActivated(ShellEvent::Pointer event);

    void ShellDeactivated(ShellEvent::Pointer event);

  private:

    WorkbenchWindow::WeakPtr window;
  };

  IShellListener::Pointer shellActivationListener;

  /**
   * Hooks a listener to track the activation and deactivation of the window's
   * shell. Notifies the active part and editor of the change
   */
  void TrackShellActivation(Shell::Pointer shell);

  struct ControlResizeListener: public GuiTk::IControlListener
  {
    ControlResizeListener(WorkbenchWindow* window);

    void ControlMoved(GuiTk::ControlEvent::Pointer e);

    void ControlResized(GuiTk::ControlEvent::Pointer e);

  private:

    void SaveBounds();

    WorkbenchWindow* window;
  };

  GuiTk::IControlListener::Pointer controlResizeListener;

  /**
   * Hooks a listener to track the resize of the window's shell. Stores the
   * new bounds if in normal state - that is, not in minimized or maximized
   * state)
   */
  void TrackShellResize(Shell::Pointer newShell);

};

}

#endif /*CHERRYWORKBENCHWINDOW_H_*/
