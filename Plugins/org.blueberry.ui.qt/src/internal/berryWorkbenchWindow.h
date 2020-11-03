/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYWORKBENCHWINDOW_H_
#define BERRYWORKBENCHWINDOW_H_

#include "berryIWorkbenchWindow.h"

#include "berryIPerspectiveListener.h"
#include "berryWindow.h"

#include "berryWorkbenchWindowConfigurer.h"
#include "berryShellPool.h"
#include "berryServiceLocator.h"
#include "berryWWinPartService.h"

#include "application/berryWorkbenchAdvisor.h"
#include "application/berryWorkbenchWindowAdvisor.h"
#include "application/berryActionBarAdvisor.h"

#include <QStringList>
#include <QRect>

namespace berry
{

struct IEvaluationReference;
struct IWorkbench;
struct IWorkbenchPage;
struct IPartService;
struct ISelectionService;
struct IPerspectiveDescriptor;
struct IWorkbenchLocationService;

class Workbench;
class WorkbenchPage;
class WWinActionBars;

/**
 * \ingroup org_blueberry_ui_qt
 *
 */
class BERRY_UI_QT WorkbenchWindow: public Window, public IWorkbenchWindow
{

public:

  /**
   * Toolbar visibility change property.
   */
  static const QString PROP_TOOLBAR_VISIBLE; // = "toolbarVisible";

  /**
   * Perspective bar visibility change property.
   */
  static const QString PROP_PERSPECTIVEBAR_VISIBLE; // = "perspectiveBarVisible";

  /**
   * The status line visibility change property.  for internal use only.
   */
  static const QString PROP_STATUS_LINE_VISIBLE; // = "statusLineVisible";

public:

  berryObjectMacro(WorkbenchWindow, Window, IWorkbenchWindow);

  WorkbenchWindow(int number);

  ~WorkbenchWindow() override;

  Object* GetService(const QString& key) override;

  bool HasService(const QString& key) const override;

  int Open();
  bool Close() override;

  Shell::Pointer GetShell() const override;

  /**
   * @see org.blueberry.ui.IPageService
   */
  void AddPerspectiveListener(IPerspectiveListener* l) override;

  /**
   * @see org.blueberry.ui.IPageService
   */
  void RemovePerspectiveListener(IPerspectiveListener* l) override;

  /**
   * @see org.blueberry.ui.IPageService
   */
  IPerspectiveListener::Events& GetPerspectiveEvents() override;

  /**
   * Returns the action bars for this window.
   */
  WWinActionBars* GetActionBars();

  SmartPointer<IWorkbenchPage> GetActivePage() const override;

  QList<SmartPointer<IWorkbenchPage> >  GetPages() const override;

  void SetPerspectiveExcludeList(const QStringList& v);
  QStringList GetPerspectiveExcludeList() const;

  void SetViewExcludeList(const QStringList& v);
  QStringList GetViewExcludeList() const;

  /**
   * Sets the active page within the window.
   *
   * @param in
   *            identifies the new active page, or <code>null</code> for no
   *            active page
   */
  void SetActivePage(SmartPointer<IWorkbenchPage> in) override;

  /**
   * Answer the menu manager for this window.
   */
  MenuManager* GetMenuManager() const;

  IWorkbench* GetWorkbench() const override;

  IPartService* GetPartService() override;

  ISelectionService* GetSelectionService() const override;

  /**
   * @return whether the tool bar should be shown. This is only applicable if
   *         the window configurer also wishes the cool bar to be visible.
   */
  bool GetToolBarVisible() const;

  /**
   * @return whether the perspective bar should be shown. This is only
   *         applicable if the window configurer also wishes the perspective
   *         bar to be visible.
   */
  bool GetPerspectiveBarVisible() const;

  /**
   * @return whether the status line should be shown. This is only applicable if
   *         the window configurer also wishes status line to be visible.
   */
  bool GetStatusLineVisible() const;

  /**
   * Add a generic property listener.
   *
   * @param listener the listener to add
   */
  void AddPropertyChangeListener(IPropertyChangeListener* listener);

  /**
   * Removes a generic property listener.
   *
   * @param listener the listener to remove
   */
  void RemovePropertyChangeListener(IPropertyChangeListener* listener);

  SmartPointer<IWorkbenchPage> OpenPage(const QString& perspectiveId,
      IAdaptable* input) override;

  SmartPointer<IWorkbenchPage> OpenPage(IAdaptable* input) override;

  //TODO menu manager
  //virtual QWidget* GetMenuManager() = 0;

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
   * update the action bars.
   */
  void UpdateActionBars();

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

  QSet<SmartPointer<IEvaluationReference> > GetMenuRestrictions() const;

protected:

  friend class WorkbenchConfigurer;
  friend class WorkbenchWindowConfigurer;
  friend class WorkbenchWindowConfigurer::WindowActionBarConfigurer;
  friend class Workbench;
  friend class LayoutPartSash;
  friend class EditorSashContainer;
  friend class WorkbenchPage;
  friend class DetachedWindow;

  /**
   * Returns the GUI dependent page composite, under which the window's
   * pages create their controls.
   */
  QWidget* GetPageComposite();

  /**
   * Creates and remembers the client composite, under which workbench pages
   * create their controls.
   */
  QWidget* CreatePageComposite(QWidget* parent);

  /**
   * Creates the contents of the workbench window, including trim controls and
   * the client composite. This MUST create the client composite via a call to
   * <code>createClientComposite</code>.
   *
   * @since 3.0
   */
  QWidget* CreateContents(Shell::Pointer parent) override;

  /**
   * Creates the default contents and layout of the shell.
   *
   * @param shell
   *            the shell
   */
  virtual void CreateDefaultContents(Shell::Pointer shell);

  void CreateTrimWidgets(SmartPointer<Shell> shell) override;

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
  WorkbenchWindowConfigurer::Pointer GetWindowConfigurer() const;

  bool CanHandleShellCloseEvent() override;

  /*
   * @see berry::Window#configureShell(Shell::Pointer)
   */
  void ConfigureShell(Shell::Pointer shell) override;

  ShellPool::Pointer GetDetachedWindowPool();

  /**
   * Fills the window's real action bars.
   *
   * @param flags
   *            indicate which bars to fill
   */
  void FillActionBars(ActionBarAdvisor::FillFlags flags);

  /**
   * The <code>WorkbenchWindow</code> implementation of this method
   * delegates to the window configurer.
   *
   * @since 3.0
   */
  QPoint GetInitialSize() override;

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
  SmartPointer<IWorkbenchPage> BusyOpenPage(const QString& perspID,
      IAdaptable* input);

  bool ClosePage(SmartPointer<IWorkbenchPage> in, bool save);

  /**
   * Makes the window visible and frontmost.
   */
  void MakeVisible();

  /**
   * The composite under which workbench pages create their controls.
   */
  QWidget* pageComposite;

private:

  /**
   * Constant indicating that all the actions bars should be filled.
   */
  static const ActionBarAdvisor::FillFlags FILL_ALL_ACTION_BARS;

  ShellPool::Pointer detachedWindowShells;

  /**
   * Object for configuring this workbench window. Lazily initialized to an
   * instance unique to this window.
   */
  mutable WorkbenchWindowConfigurer::Pointer windowConfigurer;

  WorkbenchWindowAdvisor* windowAdvisor;

  ActionBarAdvisor::Pointer actionBarAdvisor;

  SmartPointer<WWinActionBars> actionBars;

  IPropertyChangeListener::Events genericPropertyListeners;

  int number;

  /**
   * The number of large updates that are currently going on. If this is
   * number is greater than zero, then UI updateActionBars is a no-op.
   */
  int largeUpdates;

  bool closing;
  bool shellActivated;
  bool updateDisabled;

  bool toolBarVisible;
  bool perspectiveBarVisible;
  bool statusLineVisible;

  /**
   * The map of services maintained by the workbench window. These services
   * are initialized during workbench window during the
   * {@link #configureShell(Shell)}.
   */
  ServiceLocator::Pointer serviceLocator;

  QScopedPointer<IWorkbenchLocationService, QScopedPointerObjectDeleter> workbenchLocationService;

  bool emptyWindowContentsCreated;
  QWidget* emptyWindowContents;

  QRect normalBounds;

  bool asMaximizedState;

  IPerspectiveListener::Events perspectiveEvents;

  WWinPartService partService;

  QStringList perspectiveExcludeList;
  QStringList viewExcludeList;

  struct ServiceLocatorOwner: public IDisposable
  {
    ServiceLocatorOwner(WorkbenchWindow* wnd);

    WorkbenchWindow* window;

    void Dispose() override;
  };

  IDisposable::Pointer serviceLocatorOwner;

  class PageList
  {

  private:
    // List of pages in the order they were created;
    QList<SmartPointer<IWorkbenchPage> > pagesInCreationOrder;

    // List of pages where the top is the last activated.
    QList<SmartPointer<IWorkbenchPage> > pagesInActivationOrder;

    // The page explicitly activated
    SmartPointer<IWorkbenchPage> active;

  public:

    typedef QList<SmartPointer<IWorkbenchPage> >::iterator iterator;

    bool Add(SmartPointer<IWorkbenchPage> object);

    iterator Begin();

    iterator End();

    void Clear();

    bool Contains(SmartPointer<IWorkbenchPage> object);

    bool Remove(SmartPointer<IWorkbenchPage> object);

    bool IsEmpty();

    QList<SmartPointer<IWorkbenchPage> > GetPages() const;

    void SetActive(SmartPointer<IWorkbenchPage> page);

    SmartPointer<WorkbenchPage> GetActive() const;

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
      IPerspectiveDescriptor::Pointer perspective, const QString& changeId);

  /**
   * Fires perspective changed for an affected part
   */
  void FirePerspectiveChanged(SmartPointer<IWorkbenchPage> ,
      IPerspectiveDescriptor::Pointer perspective,
      IWorkbenchPartReference::Pointer partRef, const QString& changeId);

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

  struct ShellEventFilter : public QObject
  {
    ShellEventFilter(WorkbenchWindow* window);
    bool eventFilter(QObject* watched, QEvent* event) override;

  private:
    void SaveBounds(const QRect& newBounds);
    void ShellActivated();
    void ShellDeactivated();

    WorkbenchWindow* window;
  };

  ShellEventFilter resizeEventFilter;

  /**
   * Hooks a listener to track the resize of the window's shell. Stores the
   * new bounds if in normal state - that is, not in minimized or maximized
   * state)
   */
  void TrackShellResize(Shell::Pointer newShell);

  /**
   * Returns true iff we are currently deferring UI processing due to a large
   * update
   *
   * @return true iff we are deferring UI updates.
   */
  bool UpdatesDeferred() const;

  /**
   * Initializes all of the default command-based services for the workbench
   * window.
   */
  void InitializeDefaultServices();

  void FirePropertyChanged(const  QString& property, const Object::Pointer& oldValue,
                           const Object::Pointer& newValue);

};

}

#endif /*BERRYWORKBENCHWINDOW_H_*/
