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

#include "cherryIWorkbenchPage.h"
#include "cherryWorkbenchWindow.h"
#include "cherryIPerspectiveDescriptor.h"
#include "cherryUIException.h"
#include "cherryConstants.h"

#include "internal/cherryWorkbenchPlugin.h"
#include "internal/cherryWorkbenchPage.h"
#include "internal/cherryWorkbench.h"
#include "internal/cherryWorkbenchConstants.h"
#include "internal/cherryPartSite.h"
#include "internal/cherryIServiceLocatorCreator.h"

#include "tweaklets/cherryGuiWidgetsTweaklet.h"
#include "tweaklets/cherryWorkbenchTweaklet.h"

#include "services/cherryIServiceFactory.h"

#include "cherryPlatformUI.h"

#include "cherryDebugUtil.h"

namespace cherry
{

const int WorkbenchWindow::FILL_ALL_ACTION_BARS =
    ActionBarAdvisor::FILL_MENU_BAR | ActionBarAdvisor::FILL_COOL_BAR
        | ActionBarAdvisor::FILL_STATUS_LINE;

WorkbenchWindow::WorkbenchWindow(int number) :
  Window(Shell::Pointer(0)), pageComposite(0), windowAdvisor(0),
      actionBarAdvisor(0), number(number), largeUpdates(0), closing(false),
      shellActivated(false), updateDisabled(true), emptyWindowContentsCreated(
          false), emptyWindowContents(0), asMaximizedState(false), partService(
          IWorkbenchWindow::Pointer(this)), serviceLocatorOwner(
          new ServiceLocatorOwner(this))
{
  this->Register(); // increase the reference count to avoid deleting
  // this object when temporary smart pointers
  // go out of scope

  // Make sure there is a workbench. This call will throw
  // an exception if workbench not created yet.
  IWorkbench* workbench = PlatformUI::GetWorkbench();
  IServiceLocatorCreator::Pointer slc =
      workbench->GetService(IServiceLocatorCreator::GetManifestName()).Cast<
          IServiceLocatorCreator> ();

  IServiceLocator::Pointer locator(workbench);
  this->serviceLocator = slc->CreateServiceLocator(IServiceLocator::WeakPtr(
      locator), IServiceFactory::ConstPointer(0), IDisposable::WeakPtr(
      serviceLocatorOwner)).Cast<ServiceLocator> ();

  //  initializeDefaultServices();

  // Add contribution managers that are exposed to other plugins.
  //addMenuBar();
  //addCoolBar(SWT.NONE);  // style is unused
  //addStatusLine();

  this->FireWindowOpening();

  // Fill the action bars
  this->FillActionBars(FILL_ALL_ACTION_BARS);

  this->UnRegister(false); // decrease reference count and avoid deleting
  // the window
}

WorkbenchWindow::~WorkbenchWindow()
{

  Shell::Pointer shell = detachedWindowShells->availableShells.front();

  CHERRY_INFO << "Detached shell pointers: ";
  DebugUtil::PrintSmartPointerIDs(shell.GetPointer());
#ifdef OPENCHERRY_DEBUG_SMARTPOINTER
  CHERRY_INFO << " Known pointer: " << shell.GetId() << std::endl;
#endif
}

Object::Pointer WorkbenchWindow::GetService(const std::string& key)
{
  return serviceLocator->GetService(key);
}

bool WorkbenchWindow::HasService(const std::string& key) const
{
  return serviceLocator->HasService(key);
}

Shell::Pointer WorkbenchWindow::GetShell()
{
  return Window::GetShell();
}

bool WorkbenchWindow::ClosePage(IWorkbenchPage::Pointer in, bool save)
{
  // Validate the input.
  if (!pageList.Contains(in))
  {
    return false;
  }
  WorkbenchPage::Pointer oldPage = in.Cast<WorkbenchPage> ();

  // Save old perspective.
  if (save && oldPage->IsSaveNeeded())
  {
    if (!oldPage->SaveAllEditors(true))
    {
      return false;
    }
  }

  // If old page is activate deactivate.
  bool oldIsActive = (oldPage == this->GetActivePage());
  if (oldIsActive)
  {
    this->SetActivePage(IWorkbenchPage::Pointer(0));
  }

  // Close old page.
  pageList.Remove(oldPage);
  partService.PageClosed(oldPage);
  //this->FirePageClosed(oldPage);
  //oldPage->Dispose();

  // Activate new page.
  if (oldIsActive)
  {
    IWorkbenchPage::Pointer newPage = pageList.GetNextActive();
    if (newPage != 0)
    {
      this->SetActivePage(newPage);
    }
  }
  if (!closing && pageList.IsEmpty())
  {
    this->ShowEmptyWindowContents();
  }
  return true;
}

void WorkbenchWindow::AddPerspectiveListener(IPerspectiveListener::Pointer l)
{
  perspectiveEvents.AddListener(l);
}

void WorkbenchWindow::RemovePerspectiveListener(IPerspectiveListener::Pointer l)
{
  perspectiveEvents.RemoveListener(l);
}

IPerspectiveListener::Events& WorkbenchWindow::GetPerspectiveEvents()
{
  return perspectiveEvents;
}

void WorkbenchWindow::FireWindowOpening()
{
  // let the application do further configuration
  this->GetWindowAdvisor()->PreWindowOpen();
}

void WorkbenchWindow::FireWindowRestored()
{
  //StartupThreading.runWithWorkbenchExceptions(new StartupRunnable() {
  //  public void runWithException() throws Throwable {
  this->GetWindowAdvisor()->PostWindowRestore();
  //  }
  //});
}

void WorkbenchWindow::FireWindowCreated()
{
  this->GetWindowAdvisor()->PostWindowCreate();
}

void WorkbenchWindow::FireWindowOpened()
{
  this->GetWorkbenchImpl()->FireWindowOpened(IWorkbenchWindow::Pointer(this));
  this->GetWindowAdvisor()->PostWindowOpen();
}

bool WorkbenchWindow::FireWindowShellClosing()
{
  return this->GetWindowAdvisor()->PreWindowShellClose();
}

void WorkbenchWindow::FireWindowClosed()
{
  // let the application do further deconfiguration
  this->GetWindowAdvisor()->PostWindowClose();
  this->GetWorkbenchImpl()->FireWindowClosed(IWorkbenchWindow::Pointer(this));
}

///**
// * Fires page activated
// */
//void WorkbenchWindow::FirePageActivated(IWorkbenchPage::Pointer page) {
////  String label = null; // debugging only
////  if (UIStats.isDebugging(UIStats.NOTIFY_PAGE_LISTENERS)) {
////    label = "activated " + page.getLabel(); //$NON-NLS-1$
////  }
////  try {
////    UIStats.start(UIStats.NOTIFY_PAGE_LISTENERS, label);
////    UIListenerLogging.logPageEvent(this, page,
////        UIListenerLogging.WPE_PAGE_ACTIVATED);
//    pageEvents.FirePageActivated(page);
//    partService.pageActivated(page);
////  } finally {
////    UIStats.end(UIStats.NOTIFY_PAGE_LISTENERS, page.getLabel(), label);
////  }
//}
//
///**
// * Fires page closed
// */
//void WorkbenchWindow::FirePageClosed(IWorkbenchPage::Pointer page) {
//  String label = null; // debugging only
//  if (UIStats.isDebugging(UIStats.NOTIFY_PAGE_LISTENERS)) {
//    label = "closed " + page.getLabel(); //$NON-NLS-1$
//  }
//  try {
//    UIStats.start(UIStats.NOTIFY_PAGE_LISTENERS, label);
//    UIListenerLogging.logPageEvent(this, page,
//        UIListenerLogging.WPE_PAGE_CLOSED);
//    pageListeners.firePageClosed(page);
//    partService.pageClosed(page);
//  } finally {
//    UIStats.end(UIStats.NOTIFY_PAGE_LISTENERS, page.getLabel(), label);
//  }
//
//}
//
///**
// * Fires page opened
// */
//void WorkbenchWindow::FirePageOpened(IWorkbenchPage::Pointer page) {
//  String label = null; // debugging only
//  if (UIStats.isDebugging(UIStats.NOTIFY_PAGE_LISTENERS)) {
//    label = "opened " + page.getLabel(); //$NON-NLS-1$
//  }
//  try {
//    UIStats.start(UIStats.NOTIFY_PAGE_LISTENERS, label);
//    UIListenerLogging.logPageEvent(this, page,
//        UIListenerLogging.WPE_PAGE_OPENED);
//    pageListeners.firePageOpened(page);
//    partService.pageOpened(page);
//  } finally {
//    UIStats.end(UIStats.NOTIFY_PAGE_LISTENERS, page.getLabel(), label);
//  }
//}

void WorkbenchWindow::FirePerspectiveActivated(IWorkbenchPage::Pointer page,
    IPerspectiveDescriptor::Pointer perspective)
{
  //  UIListenerLogging.logPerspectiveEvent(this, page, perspective,
  //      UIListenerLogging.PLE_PERSP_ACTIVATED);
  perspectiveEvents.perspectiveActivated(page, perspective);
}

void WorkbenchWindow::FirePerspectivePreDeactivate(
    IWorkbenchPage::Pointer page, IPerspectiveDescriptor::Pointer perspective)
{
  //  UIListenerLogging.logPerspectiveEvent(this, page, perspective,
  //      UIListenerLogging.PLE_PERSP_PRE_DEACTIVATE);
  perspectiveEvents.perspectivePreDeactivate(page, perspective);
}

void WorkbenchWindow::FirePerspectiveDeactivated(IWorkbenchPage::Pointer page,
    IPerspectiveDescriptor::Pointer perspective)
{
  //  UIListenerLogging.logPerspectiveEvent(this, page, perspective,
  //      UIListenerLogging.PLE_PERSP_DEACTIVATED);
  perspectiveEvents.perspectiveDeactivated(page, perspective);
}

void WorkbenchWindow::FirePerspectiveChanged(IWorkbenchPage::Pointer page,
    IPerspectiveDescriptor::Pointer perspective, const std::string& changeId)
{
  // Some callers call this even when there is no active perspective.
  // Just ignore this case.
  if (perspective != 0)
  {
    //    UIListenerLogging.logPerspectiveChangedEvent(this, page,
    //        perspective, null, changeId);
    perspectiveEvents.perspectiveChanged(page, perspective, changeId);
  }
}

void WorkbenchWindow::FirePerspectiveChanged(IWorkbenchPage::Pointer page,
    IPerspectiveDescriptor::Pointer perspective,
    IWorkbenchPartReference::Pointer partRef, const std::string& changeId)
{
  // Some callers call this even when there is no active perspective.
  // Just ignore this case.
  if (perspective != 0)
  {
    //    UIListenerLogging.logPerspectiveChangedEvent(this, page,
    //        perspective, partRef, changeId);
    perspectiveEvents.perspectivePartChanged(page, perspective, partRef,
        changeId);
  }
}

void WorkbenchWindow::FirePerspectiveClosed(IWorkbenchPage::Pointer page,
    IPerspectiveDescriptor::Pointer perspective)
{
  //  UIListenerLogging.logPerspectiveEvent(this, page, perspective,
  //      UIListenerLogging.PLE_PERSP_CLOSED);
  perspectiveEvents.perspectiveClosed(page, perspective);
}

void WorkbenchWindow::FirePerspectiveOpened(IWorkbenchPage::Pointer page,
    IPerspectiveDescriptor::Pointer perspective)
{
  //  UIListenerLogging.logPerspectiveEvent(this, page, perspective,
  //      UIListenerLogging.PLE_PERSP_OPENED);
  perspectiveEvents.perspectiveOpened(page, perspective);
}

void WorkbenchWindow::FirePerspectiveSavedAs(IWorkbenchPage::Pointer page,
    IPerspectiveDescriptor::Pointer oldPerspective,
    IPerspectiveDescriptor::Pointer newPerspective)
{
  //  UIListenerLogging.logPerspectiveSavedAs(this, page, oldPerspective,
  //      newPerspective);
  perspectiveEvents.perspectiveSavedAs(page, oldPerspective, newPerspective);
}

void WorkbenchWindow::FillActionBars(int flags)
{
  //  Workbench workbench = getWorkbenchImpl();
  //  workbench.largeUpdateStart();
  //try {
  this->GetActionBarAdvisor()->FillActionBars(flags);

  //    final IMenuService menuService = (IMenuService) serviceLocator
  //        .getService(IMenuService.class);
  //    menuService.populateContributionManager(
  //        (ContributionManager) getActionBars().getMenuManager(),
  //        MenuUtil.MAIN_MENU);
  //    ICoolBarManager coolbar = getActionBars().getCoolBarManager();
  //    if (coolbar != null) {
  //      menuService.populateContributionManager(
  //          (ContributionManager) coolbar,
  //          MenuUtil.MAIN_TOOLBAR);
  //    }
  //
  //  } finally {
  //    workbench.largeUpdateEnd();
  //  }
}

Point WorkbenchWindow::GetInitialSize()
{
  return this->GetWindowConfigurer()->GetInitialSize();
}

bool WorkbenchWindow::Close()
{
  bool ret = false;
  //BusyIndicator.showWhile(null, new Runnable() {
  //      public void run() {
  ret = this->BusyClose();
  //      }
  //    });
  return ret;
}

bool WorkbenchWindow::BusyClose()
{
  // Whether the window was actually closed or not
  bool windowClosed = false;

  // Setup internal flags to indicate window is in
  // progress of closing and no update should be done.
  closing = true;
  updateDisabled = true;

  try
  {
    // Only do the check if it is OK to close if we are not closing
    // via the workbench as the workbench will check this itself.
    Workbench* workbench = this->GetWorkbenchImpl();
    int count = workbench->GetWorkbenchWindowCount();

    std::cout << "workbench is starting: " << workbench->IsStarting()
        << std::endl;
    std::cout << "workbench is closing: " << workbench->IsClosing()
        << std::endl;
    std::cout << "window count: " << count << std::endl;
    std::cout << "exit on last window close: "
        << workbench->GetWorkbenchConfigurer() ->GetExitOnLastWindowClose()
        << std::endl;

    // also check for starting - if the first window dies on startup
    // then we'll need to open a default window.
    if (!workbench->IsStarting() && !workbench->IsClosing() && count <= 1
        && workbench->GetWorkbenchConfigurer() ->GetExitOnLastWindowClose())
    {
      std::cout << "Calling Workbench::Close()" << std::endl;
      windowClosed = workbench->Close();
    }
    else
    {
      if (this->OkToClose())
      {
        std::cout << "Calling this->HardClose()" << std::endl;
        windowClosed = this->HardClose();
      }
    }
  } catch (std::exception& exc)
  {
    if (!windowClosed)
    {
      // Reset the internal flags if window was not closed.
      closing = false;
      updateDisabled = false;
    }

    throw exc;
  }

  //    if (windowClosed && tracker != null) {
  //      tracker.close();
  //    }

  return windowClosed;
}

void WorkbenchWindow::MakeVisible()
{
  Shell::Pointer shell = GetShell();
  if (shell)
  {
    // see bug 96700 and bug 4414 for a discussion on the use of open()
    // here
    shell->Open();
  }
}

bool WorkbenchWindow::OkToClose()
{
  // Save all of the editors.
  if (!this->GetWorkbenchImpl()->IsClosing())
  {
    if (!this->SaveAllPages(true))
    {
      return false;
    }
  }
  return true;
}

bool WorkbenchWindow::SaveAllPages(bool bConfirm)
{
  bool bRet = true;
  PageList::iterator itr = pageList.Begin();
  while (bRet && itr != pageList.End())
  {
    bRet = (*itr)->SaveAllEditors(bConfirm);
    ++itr;
  }
  return bRet;
}

bool WorkbenchWindow::HardClose()
{
  std::exception exc;
  bool exceptionOccured = false;

  try
  {
    // Clear the action sets, fix for bug 27416.
    //getActionPresentation().clearActionSets();

    // Remove the handler submissions. Bug 64024.
    /*
     final IWorkbench workbench = getWorkbench();
     final IHandlerService handlerService = (IHandlerService) workbench.getService(IHandlerService.class);
     handlerService.deactivateHandlers(handlerActivations);
     final Iterator activationItr = handlerActivations.iterator();
     while (activationItr.hasNext()) {
     final IHandlerActivation activation = (IHandlerActivation) activationItr
     .next();
     activation.getHandler().dispose();
     }
     handlerActivations.clear();
     globalActionHandlersByCommandId.clear();
     */

    // Remove the enabled submissions. Bug 64024.
    /*
     final IContextService contextService = (IContextService) workbench.getService(IContextService.class);
     contextService.unregisterShell(getShell());
     */

    this->CloseAllPages();

    this->FireWindowClosed();

    // time to wipe out our populate
    /*
     IMenuService menuService = (IMenuService) workbench
     .getService(IMenuService.class);
     menuService
     .releaseContributions(((ContributionManager) getActionBars()
     .getMenuManager()));
     ICoolBarManager coolbar = getActionBars().getCoolBarManager();
     if (coolbar != null) {
     menuService
     .releaseContributions(((ContributionManager) coolbar));
     }
     */

    //getActionBarAdvisor().dispose();
    //getWindowAdvisor().dispose();
    //detachedWindowShells.dispose();
    delete windowAdvisor;
    windowAdvisor = 0;

    // Null out the progress region. Bug 64024.
    //progressRegion = null;

    // Remove drop targets
    /*
     DragUtil.removeDragTarget(null, trimDropTarget);
     DragUtil.removeDragTarget(getShell(), trimDropTarget);
     trimDropTarget = null;

     if (trimMgr2 != null) {
     trimMgr2.dispose();
     trimMgr2 = null;
     }

     if (trimContributionMgr != null) {
     trimContributionMgr.dispose();
     trimContributionMgr = null;
     }
     */
  } catch (std::exception& e)
  {
    exc = e;
    exceptionOccured = true;
  }

  bool result = Window::Close();
  // Bring down all of the services ... after the window goes away
  serviceLocator->Dispose();
  //menuRestrictions.clear();

  if (exceptionOccured)
    throw exc;

  return result;
}

void WorkbenchWindow::CloseAllPages()
{
  // Deactivate active page.
  this->SetActivePage(IWorkbenchPage::Pointer(0));

  // Clone and deref all so that calls to getPages() returns
  // empty list (if called by pageClosed event handlers)
  PageList oldList = pageList;
  pageList.Clear();

  // Close all.
  for (PageList::iterator itr = oldList.Begin(); itr != oldList.End(); ++itr)
  {
    partService.PageClosed(*itr);
    //(*itr)->FirePageClosed(page);
    //page.dispose();
  }
  if (!closing)
  {
    //this->ShowEmptyWindowContents();
  }
}

IWorkbenchPage::Pointer WorkbenchWindow::GetActivePage()
{
  return pageList.GetActive();
}

IWorkbench* WorkbenchWindow::GetWorkbench()
{
  return PlatformUI::GetWorkbench();
}

IPartService* WorkbenchWindow::GetPartService()
{
  return &partService;
}

ISelectionService* WorkbenchWindow::GetSelectionService()
{
  return partService.GetSelectionService();
}

bool WorkbenchWindow::IsClosing()
{
  return closing || this->GetWorkbenchImpl()->IsClosing();
}

int WorkbenchWindow::Open()
{
  //  if (getPages().length == 0) {
  //    showEmptyWindowContents();
  //  }
  this->FireWindowCreated();
  this->GetWindowAdvisor()->OpenIntro();

  int result = Window::Open();

  // It's time for a layout ... to insure that if TrimLayout
  // is in play, it updates all of the trim it's responsible
  // for. We have to do this before updating in order to get
  // the PerspectiveBar management correct...see defect 137334
  //getShell().layout();

  this->FireWindowOpened();
  //  if (perspectiveSwitcher != null) {
  //    perspectiveSwitcher.updatePerspectiveBar();
  //    perspectiveSwitcher.updateBarParent();
  //  }

  return result;
}

void* WorkbenchWindow::GetPageComposite()
{
  return pageComposite;
}

void* WorkbenchWindow::CreateContents(Shell::Pointer parent)
{
  // we know from Window.create that the parent is a Shell.
  this->GetWindowAdvisor()->CreateWindowContents(parent);
  // the page composite must be set by createWindowContents
  poco_assert(pageComposite != 0)
; // "createWindowContents must call configurer.createPageComposite"); //$NON-NLS-1$
  return pageComposite;
}

void WorkbenchWindow::CreateDefaultContents(Shell::Pointer shell)
{
  //pageComposite = Tweaklets::Get(WorkbenchTweaklet::KEY)->CreateDefaultWindowContents(shell);
  this->CreatePageComposite(shell->GetControl());
}

bool WorkbenchWindow::UnableToRestorePage(IMemento::Pointer pageMem)
{
  std::string pageName;
  pageMem->GetString(WorkbenchConstants::TAG_LABEL, pageName);

  //  return new Status(IStatus.ERROR, PlatformUI.PLUGIN_ID, 0, NLS.bind(
  //      WorkbenchMessages.WorkbenchWindow_unableToRestorePerspective,
  //      pageName), null);
  WorkbenchPlugin::Log("Unable to restore perspective: " + pageName);
  return false;
}

bool WorkbenchWindow::RestoreState(IMemento::Pointer memento,
    IPerspectiveDescriptor::Pointer activeDescriptor)
{
  //TODO WorkbenchWindow restore state
  poco_assert(GetShell());

  //  final MultiStatus result = new MultiStatus(PlatformUI.PLUGIN_ID, IStatus.OK,
  //      WorkbenchMessages.WorkbenchWindow_problemsRestoringWindow, null);
  bool result = true;

  // Restore the window advisor state.
  IMemento::Pointer windowAdvisorState = memento
  ->GetChild(WorkbenchConstants::TAG_WORKBENCH_WINDOW_ADVISOR);
  if (windowAdvisorState)
  {
    //result.add(getWindowAdvisor().restoreState(windowAdvisorState));
    result &= GetWindowAdvisor()->RestoreState(windowAdvisorState);
  }

  // Restore actionbar advisor state.
  IMemento::Pointer actionBarAdvisorState = memento
  ->GetChild(WorkbenchConstants::TAG_ACTION_BAR_ADVISOR);
  if (actionBarAdvisorState)
  {
    //    result.add(getActionBarAdvisor()
    //        .restoreState(actionBarAdvisorState));
    result &= GetActionBarAdvisor()
    ->RestoreState(actionBarAdvisorState);
  }

  // Read window's bounds and state.
  Rectangle displayBounds;
  //  StartupThreading.runWithoutExceptions(new StartupRunnable() {
  //
  //    public void runWithException() {
  displayBounds = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetScreenSize();
  //displayBounds = GetShell()->GetDisplay()->GetBounds();

  //    }});
  Rectangle shellBounds;

  //  final IMemento fastViewMem = memento
  //      .getChild(IWorkbenchConstants.TAG_FAST_VIEW_DATA);
  //  if (fastViewMem != null) {
  //    if (fastViewBar != null) {
  //      StartupThreading.runWithoutExceptions(new StartupRunnable() {
  //
  //        public void runWithException() {
  //          fastViewBar.restoreState(fastViewMem);
  //        }});
  //
  //    }
  //  }

  memento->GetInteger(WorkbenchConstants::TAG_X, shellBounds.x);
  memento->GetInteger(WorkbenchConstants::TAG_Y, shellBounds.y);
  memento->GetInteger(WorkbenchConstants::TAG_WIDTH, shellBounds.width);
  memento->GetInteger(WorkbenchConstants::TAG_HEIGHT, shellBounds.height);
  if (!shellBounds.IsEmpty())
  {
    //    StartupThreading.runWithoutExceptions(new StartupRunnable() {
    //
    //      public void runWithException() {
    if (!shellBounds.Intersects(displayBounds))
    {
      Rectangle clientArea(Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetAvailableScreenSize());
      shellBounds.x = clientArea.x;
      shellBounds.y = clientArea.y;
    }
    GetShell()->SetBounds(shellBounds);
    //      }});
  }
  std::string maximized; memento->GetString(WorkbenchConstants::TAG_MAXIMIZED, maximized);
  if (maximized == "true")
  {
    //    StartupThreading.runWithoutExceptions(new StartupRunnable() {
    //
    //      public void runWithException() {
    GetShell()->SetMaximized(true);
    //      }});

  }
  std::string minimized; memento->GetString(WorkbenchConstants::TAG_MINIMIZED, minimized);
  if (minimized == "true")
  {
    // getShell().setMinimized(true);
  }

  //  // restore the width of the perspective bar
  //  if (perspectiveSwitcher != null) {
  //    perspectiveSwitcher.restoreState(memento);
  //  }

  //  // Restore the cool bar order by creating all the tool bar contribution
  //  // items
  //  // This needs to be done before pages are created to ensure proper
  //  // canonical creation
  //  // of cool items
  //  final ICoolBarManager2 coolBarMgr = (ICoolBarManager2) getCoolBarManager2();
  //      if (coolBarMgr != null) {
  //    IMemento coolBarMem = memento
  //        .getChild(IWorkbenchConstants.TAG_COOLBAR_LAYOUT);
  //    if (coolBarMem != null) {
  //      // Check if the layout is locked
  //      final Integer lockedInt = coolBarMem
  //          .getInteger(IWorkbenchConstants.TAG_LOCKED);
  //      StartupThreading.runWithoutExceptions(new StartupRunnable(){
  //
  //        public void runWithException() {
  //          if ((lockedInt != null) && (lockedInt.intValue() == 1)) {
  //            coolBarMgr.setLockLayout(true);
  //          } else {
  //            coolBarMgr.setLockLayout(false);
  //          }
  //        }});
  //
  //      // The new layout of the cool bar manager
  //      ArrayList coolBarLayout = new ArrayList();
  //      // Traverse through all the cool item in the memento
  //      IMemento contributionMems[] = coolBarMem
  //          .getChildren(IWorkbenchConstants.TAG_COOLITEM);
  //      for (int i = 0; i < contributionMems.length; i++) {
  //        IMemento contributionMem = contributionMems[i];
  //        String type = contributionMem
  //            .getString(IWorkbenchConstants.TAG_ITEM_TYPE);
  //        if (type == null) {
  //          // Do not recognize that type
  //          continue;
  //        }
  //        String id = contributionMem
  //            .getString(IWorkbenchConstants.TAG_ID);
  //
  //        // Prevent duplicate items from being read back in.
  //        IContributionItem existingItem = coolBarMgr.find(id);
  //        if ((id != null) && (existingItem != null)) {
  //          if (Policy.DEBUG_TOOLBAR_DISPOSAL) {
  //            System.out
  //                .println("Not loading duplicate cool bar item: " + id); //$NON-NLS-1$
  //          }
  //          coolBarLayout.add(existingItem);
  //          continue;
  //        }
  //        IContributionItem newItem = null;
  //        if (type.equals(IWorkbenchConstants.TAG_TYPE_SEPARATOR)) {
  //          if (id != null) {
  //            newItem = new Separator(id);
  //          } else {
  //            newItem = new Separator();
  //          }
  //        } else if (id != null) {
  //          if (type
  //              .equals(IWorkbenchConstants.TAG_TYPE_GROUPMARKER)) {
  //            newItem = new GroupMarker(id);
  //
  //          } else if (type
  //              .equals(IWorkbenchConstants.TAG_TYPE_TOOLBARCONTRIBUTION)
  //              || type
  //                  .equals(IWorkbenchConstants.TAG_TYPE_PLACEHOLDER)) {
  //
  //            // Get Width and height
  //            Integer width = contributionMem
  //                .getInteger(IWorkbenchConstants.TAG_ITEM_X);
  //            Integer height = contributionMem
  //                .getInteger(IWorkbenchConstants.TAG_ITEM_Y);
  //            // Look for the object in the current cool bar
  //            // manager
  //            IContributionItem oldItem = coolBarMgr.find(id);
  //            // If a tool bar contribution item already exists
  //            // for this id then use the old object
  //            if (oldItem != null) {
  //              newItem = oldItem;
  //            } else {
  //              IActionBarPresentationFactory actionBarPresentation = getActionBarPresentationFactory();
  //              newItem = actionBarPresentation.createToolBarContributionItem(
  //                  actionBarPresentation.createToolBarManager(), id);
  //              if (type
  //                  .equals(IWorkbenchConstants.TAG_TYPE_PLACEHOLDER)) {
  //                IToolBarContributionItem newToolBarItem = (IToolBarContributionItem) newItem;
  //                if (height != null) {
  //                  newToolBarItem.setCurrentHeight(height
  //                      .intValue());
  //                }
  //                if (width != null) {
  //                  newToolBarItem.setCurrentWidth(width
  //                      .intValue());
  //                }
  //                newItem = new PlaceholderContributionItem(
  //                    newToolBarItem);
  //              }
  //              // make it invisible by default
  //              newItem.setVisible(false);
  //              // Need to add the item to the cool bar manager
  //              // so that its canonical order can be preserved
  //              IContributionItem refItem = findAlphabeticalOrder(
  //                  IWorkbenchActionConstants.MB_ADDITIONS,
  //                  id, coolBarMgr);
  //              if (refItem != null) {
  //                coolBarMgr.insertAfter(refItem.getId(),
  //                    newItem);
  //              } else {
  //                coolBarMgr.add(newItem);
  //              }
  //            }
  //            // Set the current height and width
  //            if ((width != null)
  //                && (newItem instanceof IToolBarContributionItem)) {
  //              ((IToolBarContributionItem) newItem)
  //                  .setCurrentWidth(width.intValue());
  //            }
  //            if ((height != null)
  //                && (newItem instanceof IToolBarContributionItem)) {
  //              ((IToolBarContributionItem) newItem)
  //                  .setCurrentHeight(height.intValue());
  //            }
  //          }
  //        }
  //        // Add new item into cool bar manager
  //        if (newItem != null) {
  //          coolBarLayout.add(newItem);
  //          newItem.setParent(coolBarMgr);
  //          coolBarMgr.markDirty();
  //        }
  //      }
  //
  //      // We need to check if we have everything we need in the layout.
  //      boolean newlyAddedItems = false;
  //      IContributionItem[] existingItems = coolBarMgr.getItems();
  //      for (int i = 0; i < existingItems.length && !newlyAddedItems; i++) {
  //        IContributionItem existingItem = existingItems[i];
  //
  //        /*
  //         * This line shouldn't be necessary, but is here for
  //         * robustness.
  //         */
  //        if (existingItem == null) {
  //          continue;
  //        }
  //
  //        boolean found = false;
  //        Iterator layoutItemItr = coolBarLayout.iterator();
  //        while (layoutItemItr.hasNext()) {
  //          IContributionItem layoutItem = (IContributionItem) layoutItemItr
  //              .next();
  //          if ((layoutItem != null)
  //              && (layoutItem.equals(existingItem))) {
  //            found = true;
  //            break;
  //          }
  //        }
  //
  //        if (!found) {
  //          if (existingItem != null) {
  //            newlyAddedItems = true;
  //          }
  //        }
  //      }
  //
  //      // Set the cool bar layout to the given layout.
  //      if (!newlyAddedItems) {
  //        final IContributionItem[] itemsToSet = new IContributionItem[coolBarLayout
  //            .size()];
  //        coolBarLayout.toArray(itemsToSet);
  //        StartupThreading
  //            .runWithoutExceptions(new StartupRunnable() {
  //
  //              public void runWithException() {
  //                coolBarMgr.setItems(itemsToSet);
  //              }
  //            });
  //      }
  //
  //    } else {
  //      // For older workbenchs
  //      coolBarMem = memento
  //          .getChild(IWorkbenchConstants.TAG_TOOLBAR_LAYOUT);
  //      if (coolBarMem != null) {
  //        // Restore an older layout
  //        restoreOldCoolBar(coolBarMem);
  //      }
  //    }
  //  }

  // Recreate each page in the window.
  IWorkbenchPage::Pointer newActivePage;
  std::vector<IMemento::Pointer> pageArray = memento
  ->GetChildren(WorkbenchConstants::TAG_PAGE);
  for (std::size_t i = 0; i < pageArray.size(); i++)
  {
    IMemento::Pointer pageMem = pageArray[i];
    std::string strFocus; pageMem->GetString(WorkbenchConstants::TAG_FOCUS, strFocus);
    if (strFocus.empty())
    {
      continue;
    }

    // Get the input factory.
    IAdaptable* input = 0;
    IMemento::Pointer inputMem = pageMem->GetChild(WorkbenchConstants::TAG_INPUT);
    if (inputMem)
    {
      std::string factoryID; inputMem->GetString(WorkbenchConstants::TAG_FACTORY_ID, factoryID);
      if (factoryID.empty())
      {
        WorkbenchPlugin
        ::Log("Unable to restore page - no input factory ID.");
        //result.add(unableToRestorePage(pageMem));
        result &= UnableToRestorePage(pageMem);
        continue;
      }
      //      try {
      //        UIStats.start(UIStats.RESTORE_WORKBENCH,
      //            "WorkbenchPageFactory"); //$NON-NLS-1$
      //        StartupThreading
      //            .runWithoutExceptions(new StartupRunnable() {
      //
      //              public void runWithException() throws Throwable {
      //                IElementFactory factory = PlatformUI
      //                    .getWorkbench().getElementFactory(
      //                        factoryID);
      //                if (factory == null) {
      //                  WorkbenchPlugin
      //                      .log("Unable to restore page - cannot instantiate input factory: " + factoryID); //$NON-NLS-1$
      //                  result
      //                      .add(unableToRestorePage(pageMem));
      //                  return;
      //                }
      //
      //                // Get the input element.
      //                input[0] = factory.createElement(inputMem);
      //              }
      //            });
      //
      //        if (input[0] == null) {
      //          WorkbenchPlugin
      //              .log("Unable to restore page - cannot instantiate input element: " + factoryID); //$NON-NLS-1$
      //          result.add(unableToRestorePage(pageMem));
      //          continue;
      //        }
      //      } finally {
      //        UIStats.end(UIStats.RESTORE_WORKBENCH, factoryID,
      //            "WorkbenchPageFactory"); //$NON-NLS-1$
      //      }
    }
    // Open the perspective.
    IAdaptable* finalInput = input;
    WorkbenchPage::Pointer newPage;
    try
    {
      //      StartupThreading.runWithWorkbenchExceptions(new StartupRunnable(){
      //
      //        public void runWithException() throws WorkbenchException {
      newPage = new WorkbenchPage(this, finalInput);
      //        }});

      //result.add(newPage[0].restoreState(pageMem, activeDescriptor));
      result &= newPage->RestoreState(pageMem, activeDescriptor);
      pageList.Add(newPage);
      //      StartupThreading.runWithoutExceptions(new StartupRunnable() {
      //
      //        public void runWithException() throws Throwable {
      partService.PageOpened(newPage);
      //          firePageOpened(newPage[0]);
      //        }});

    }
    catch (const WorkbenchException& e)
    {
      WorkbenchPlugin::Log(
          "Unable to restore perspective - constructor failed.", e); //$NON-NLS-1$
      //result.add(e.getStatus());
      continue;
    }

    if (!strFocus.empty())
    {
      newActivePage = newPage;
    }
  }

  // If there are no pages create a default.
  if (pageList.IsEmpty())
  {
    try
    {
      const std::string defPerspID = this->GetWorkbenchImpl()->GetPerspectiveRegistry()
      ->GetDefaultPerspective();
      if (!defPerspID.empty())
      {
        WorkbenchPage::Pointer newPage;
        //StartupThreading.runWithWorkbenchExceptions(new StartupRunnable() {

        //  public void runWithException() throws Throwable {
        newPage = new WorkbenchPage(this, defPerspID,
            this->GetDefaultPageInput());
        //  }});

        pageList.Add(newPage);
        //StartupThreading.runWithoutExceptions(new StartupRunnable() {

        //  public void runWithException() throws Throwable {
        //    firePageOpened(newPage[0]);
        partService.PageOpened(newPage);
        //  }});
      }
    }
    catch (WorkbenchException& e)
    {
      WorkbenchPlugin
      ::Log(
          "Unable to create default perspective - constructor failed.", e);
      result = false;
      //TODO set product name
      //      String productName = WorkbenchPlugin.getDefault()
      //          .getProductName();
      //      if (productName == null) {
      //        productName = ""; //$NON-NLS-1$
      //      }
      //      getShell().setText(productName);
    }
  }

  // Set active page.
  if (newActivePage.IsNull())
  {
    newActivePage = pageList.GetNextActive().Cast<IWorkbenchPage>();
  }

  //StartupThreading.runWithoutExceptions(new StartupRunnable() {

  //  public void runWithException() throws Throwable {
  this->SetActivePage(newActivePage);
  //  }});


  //  final IMemento introMem = memento.getChild(IWorkbenchConstants.TAG_INTRO);
  //  if (introMem != null) {
  //    StartupThreading.runWithoutExceptions(new StartupRunnable() {
  //
  //      public void runWithException() throws Throwable {
  //        getWorkbench()
  //            .getIntroManager()
  //            .showIntro(
  //                WorkbenchWindow.this,
  //                Boolean
  //                    .valueOf(
  //                        introMem
  //                            .getString(IWorkbenchConstants.TAG_STANDBY))
  //                    .booleanValue());
  //      }
  //    });
  //
  //  }
  //
  //  // Only restore the trim state if we're using the default layout
  //  if (defaultLayout != null) {
  //    // Restore the trim state. We pass in the 'root'
  //    // memento since we have to check for pre-3.2
  //    // state.
  //    result.add(restoreTrimState(memento));
  //  }

  return result;
}

IAdaptable* WorkbenchWindow::GetDefaultPageInput()
{
  return this->GetWorkbenchImpl()->GetDefaultPageInput();
}

IWorkbenchPage::Pointer WorkbenchWindow::OpenPage(
    const std::string& perspId, IAdaptable* input)
{
  // Run op in busy cursor.
  IWorkbenchPage::Pointer result;
  //BusyIndicator.showWhile(null, new Runnable() {
  //  public void run() {
  result = this->BusyOpenPage(perspId, input);

  // }

  return result;
}

SmartPointer<IWorkbenchPage> WorkbenchWindow::OpenPage(IAdaptable* input)
{
  std::string perspId = this->GetWorkbenchImpl()->GetDefaultPerspectiveId();
  return this->OpenPage(perspId, input);
}

IWorkbenchPage::Pointer WorkbenchWindow::BusyOpenPage(
    const std::string& perspID, IAdaptable* input)
{
  IWorkbenchPage::Pointer newPage;

  if (pageList.IsEmpty())
  {
    newPage = new WorkbenchPage(this, perspID, input);
    pageList.Add(newPage);
    //this->FirePageOpened(newPage);
    partService.PageOpened(newPage);
    this->SetActivePage(newPage);
  }
  else
  {
    IWorkbenchWindow::Pointer window = this->GetWorkbench()->OpenWorkbenchWindow(perspID, input);
    newPage = window->GetActivePage();
  }

  return newPage;
}

int WorkbenchWindow::GetNumber()
{
  return number;
}

void WorkbenchWindow::LargeUpdateStart()
{
  largeUpdates++;
}

void WorkbenchWindow::LargeUpdateEnd()
{
  if (--largeUpdates == 0)
  {
    //TODO WorkbenchWindow update action bars after large update
    //this->UpdateActionBars();
  }
}

void WorkbenchWindow::SetActivePage(IWorkbenchPage::Pointer in)
{
  if (this->GetActivePage() == in)
  {
    return;
  }

  // 1FVGTNR: ITPUI:WINNT - busy cursor for switching perspectives
  //BusyIndicator.showWhile(getShell().getDisplay(), new Runnable() {
  //  public void run() {
  // Deactivate old persp.
  WorkbenchPage::Pointer currentPage = pageList.GetActive();
  if (currentPage.IsNotNull())
  {
    currentPage->OnDeactivate();
  }

  // Activate new persp.
  if (in.IsNull() || pageList.Contains(in))
  {
    pageList.SetActive(in);
  }
  WorkbenchPage::Pointer newPage = pageList.GetActive();
  //Composite parent = getPageComposite();
  //StackLayout layout = (StackLayout) parent.getLayout();
  if (newPage.IsNotNull())
  {
    //layout.topControl = newPage.getClientComposite();
    //parent.layout();
    this->HideEmptyWindowContents();
    newPage->OnActivate();
    //this->FirePageActivated(newPage);
    partService.PageActivated(newPage);
    //TODO perspective
    if (newPage->GetPerspective() != 0)
    {
      this->FirePerspectiveActivated(newPage, newPage->GetPerspective());
    }
  }
  else
  {
    //layout.topControl = null;
    //parent.layout();
  }

  //updateFastViewBar();

  if (this->IsClosing())
  {
    return;
  }

  updateDisabled = false;

  // Update action bars ( implicitly calls updateActionBars() )
  //updateActionSets();
  //submitGlobalActions();

  //if (perspectiveSwitcher != null) {
  //  perspectiveSwitcher.update(false);
  //}

  //getMenuManager().update(IAction.TEXT);
  //  }
  //});
}

bool WorkbenchWindow::SaveState(IMemento::Pointer memento)
{
  //  MultiStatus result = new MultiStatus(PlatformUI.PLUGIN_ID, IStatus.OK,
  //        WorkbenchMessages.WorkbenchWindow_problemsSavingWindow, null);
  bool result = true;

  // Save the window's state and bounds.
  if (GetShell()->GetMaximized() || asMaximizedState)
  {
    memento->PutString(WorkbenchConstants::TAG_MAXIMIZED, "true");
  }
  if (GetShell()->GetMinimized())
  {
    memento->PutString(WorkbenchConstants::TAG_MINIMIZED, "true");
  }
  if (normalBounds.IsEmpty())
  {
    normalBounds = GetShell()->GetBounds();
  }

  //  IMemento fastViewBarMem = memento
  //      .createChild(IWorkbenchConstants.TAG_FAST_VIEW_DATA);
  //  if (fastViewBar != null) {
  //    fastViewBar.saveState(fastViewBarMem);
  //  }

  memento->PutInteger(WorkbenchConstants::TAG_X, normalBounds.x);
  memento->PutInteger(WorkbenchConstants::TAG_Y, normalBounds.y);
  memento->PutInteger(WorkbenchConstants::TAG_WIDTH, normalBounds.width);
  memento->PutInteger(WorkbenchConstants::TAG_HEIGHT, normalBounds.height);

  //  IWorkbenchPage::Pointer activePage = GetActivePage();
  //  if (activePage
  //      && activePage->FindView(IIntroConstants.INTRO_VIEW_ID) != null) {
  //    IMemento introMem = memento
  //        .createChild(IWorkbenchConstants.TAG_INTRO);
  //    boolean isStandby = getWorkbench()
  //        .getIntroManager()
  //        .isIntroStandby(getWorkbench().getIntroManager().getIntro());
  //    introMem.putString(IWorkbenchConstants.TAG_STANDBY, String
  //        .valueOf(isStandby));
  //  }

  //  // save the width of the perspective bar
  //  IMemento persBarMem = memento
  //      .createChild(IWorkbenchConstants.TAG_PERSPECTIVE_BAR);
  //  if (perspectiveSwitcher != null) {
  //    perspectiveSwitcher.saveState(persBarMem);
  //  }

  //  // / Save the order of the cool bar contribution items
  //        ICoolBarManager2 coolBarMgr = (ICoolBarManager2) getCoolBarManager2();
  //        if (coolBarMgr != null) {
  //          coolBarMgr.refresh();
  //      IMemento coolBarMem = memento
  //          .createChild(IWorkbenchConstants.TAG_COOLBAR_LAYOUT);
  //            if (coolBarMgr.getLockLayout() == true) {
  //        coolBarMem.putInteger(IWorkbenchConstants.TAG_LOCKED, 1);
  //      } else {
  //        coolBarMem.putInteger(IWorkbenchConstants.TAG_LOCKED, 0);
  //      }
  //            IContributionItem[] items = coolBarMgr.getItems();
  //      for (int i = 0; i < items.length; i++) {
  //        IMemento coolItemMem = coolBarMem
  //            .createChild(IWorkbenchConstants.TAG_COOLITEM);
  //        IContributionItem item = items[i];
  //        // The id of the contribution item
  //        if (item.getId() != null) {
  //          coolItemMem.putString(IWorkbenchConstants.TAG_ID, item
  //              .getId());
  //        }
  //        // Write out type and size if applicable
  //        if (item.isSeparator()) {
  //          coolItemMem.putString(IWorkbenchConstants.TAG_ITEM_TYPE,
  //              IWorkbenchConstants.TAG_TYPE_SEPARATOR);
  //        } else if (item.isGroupMarker() && !item.isSeparator()) {
  //          coolItemMem.putString(IWorkbenchConstants.TAG_ITEM_TYPE,
  //              IWorkbenchConstants.TAG_TYPE_GROUPMARKER);
  //        } else {
  //          if (item instanceof PlaceholderContributionItem) {
  //            coolItemMem.putString(
  //                IWorkbenchConstants.TAG_ITEM_TYPE,
  //                IWorkbenchConstants.TAG_TYPE_PLACEHOLDER);
  //          } else {
  //            // Store the identifier.
  //            coolItemMem
  //                .putString(
  //                    IWorkbenchConstants.TAG_ITEM_TYPE,
  //                    IWorkbenchConstants.TAG_TYPE_TOOLBARCONTRIBUTION);
  //          }
  //
  //          /*
  //           * Retrieve a reasonable approximation of the height and
  //           * width, if possible.
  //           */
  //          final int height;
  //          final int width;
  //          if (item instanceof IToolBarContributionItem) {
  //            IToolBarContributionItem toolBarItem = (IToolBarContributionItem) item;
  //            toolBarItem.saveWidgetState();
  //            height = toolBarItem.getCurrentHeight();
  //            width = toolBarItem.getCurrentWidth();
  //          } else if (item instanceof PlaceholderContributionItem) {
  //            PlaceholderContributionItem placeholder = (PlaceholderContributionItem) item;
  //            height = placeholder.getHeight();
  //            width = placeholder.getWidth();
  //          } else {
  //            height = -1;
  //            width = -1;
  //          }
  //
  //          // Store the height and width.
  //          coolItemMem.putInteger(IWorkbenchConstants.TAG_ITEM_X,
  //              width);
  //          coolItemMem.putInteger(IWorkbenchConstants.TAG_ITEM_Y,
  //              height);
  //        }
  //      }
  //    }


  // Save each page.
  for (PageList::iterator itr = pageList.Begin();
      itr != pageList.End(); ++itr)
  {
    WorkbenchPage::Pointer page = itr->Cast<WorkbenchPage>();

    // Save perspective.
    IMemento::Pointer pageMem = memento
    ->CreateChild(WorkbenchConstants::TAG_PAGE);
    pageMem->PutString(WorkbenchConstants::TAG_LABEL, page->GetLabel());
    //result.add(page.saveState(pageMem));
    result &= page->SaveState(pageMem);

    if (page == GetActivePage().Cast<WorkbenchPage>())
    {
      pageMem->PutString(WorkbenchConstants::TAG_FOCUS, "true");
    }

    //    // Get the input.
    //    IAdaptable* input = page->GetInput();
    //    if (input != 0) {
    //      IPersistableElement persistable = (IPersistableElement) Util.getAdapter(input,
    //          IPersistableElement.class);
    //      if (persistable == null) {
    //        WorkbenchPlugin
    //            .log("Unable to save page input: " //$NON-NLS-1$
    //                + input
    //                + ", because it does not adapt to IPersistableElement"); //$NON-NLS-1$
    //      } else {
    //        // Save input.
    //        IMemento inputMem = pageMem
    //            .createChild(IWorkbenchConstants.TAG_INPUT);
    //        inputMem.putString(IWorkbenchConstants.TAG_FACTORY_ID,
    //            persistable.getFactoryId());
    //        persistable.saveState(inputMem);
    //      }
    //    }
  }

  // Save window advisor state.
  IMemento::Pointer windowAdvisorState = memento
  ->CreateChild(WorkbenchConstants::TAG_WORKBENCH_WINDOW_ADVISOR);
  //result.add(getWindowAdvisor().saveState(windowAdvisorState));
  result &= GetWindowAdvisor()->SaveState(windowAdvisorState);

  // Save actionbar advisor state.
  IMemento::Pointer actionBarAdvisorState = memento
  ->CreateChild(WorkbenchConstants::TAG_ACTION_BAR_ADVISOR);
  //result.add(getActionBarAdvisor().saveState(actionBarAdvisorState));
  result &= GetActionBarAdvisor()->SaveState(actionBarAdvisorState);

  //  // Only save the trim state if we're using the default layout
  //  if (defaultLayout != null) {
  //    IMemento trimState = memento.createChild(IWorkbenchConstants.TAG_TRIM);
  //    result.add(saveTrimState(trimState));
  //  }

  return result;
}

WorkbenchWindowConfigurer::Pointer WorkbenchWindow::GetWindowConfigurer()
{
  if (windowConfigurer.IsNull())
  {
    // lazy initialize
    windowConfigurer = new WorkbenchWindowConfigurer(WorkbenchWindow::Pointer(this));
  }
  return windowConfigurer;
}

void WorkbenchWindow::ConfigureShell(Shell::Pointer shell)
{
  Window::ConfigureShell(shell);

  detachedWindowShells = new ShellPool(shell, Constants::TITLE
      | Constants::MAX | Constants::CLOSE | Constants::RESIZE | Constants::BORDER );

  std::string title = this->GetWindowConfigurer()->BasicGetTitle();
  if (title != "")
  {
    shell->SetText(title);
  }

  //  final IWorkbench workbench = getWorkbench();
  //  workbench.getHelpSystem().setHelp(shell,
  //      IWorkbenchHelpContextIds.WORKBENCH_WINDOW);

  //    final IContextService contextService = (IContextService) getWorkbench().getService(IContextService.class);
  //    contextService.registerShell(shell, IContextService.TYPE_WINDOW);

  this->TrackShellActivation(shell);
  this->TrackShellResize(shell);
}

ShellPool::Pointer WorkbenchWindow::GetDetachedWindowPool()
{
  return detachedWindowShells;
}

WorkbenchAdvisor* WorkbenchWindow::GetAdvisor()
{
  return this->GetWorkbenchImpl()->GetAdvisor();
}

WorkbenchWindowAdvisor* WorkbenchWindow::GetWindowAdvisor()
{
  if (windowAdvisor == 0)
  {
    windowAdvisor = this->GetAdvisor()->CreateWorkbenchWindowAdvisor(this->GetWindowConfigurer());
    poco_check_ptr(windowAdvisor);
  }
  return windowAdvisor;
}

ActionBarAdvisor::Pointer WorkbenchWindow::GetActionBarAdvisor()
{
  if (actionBarAdvisor.IsNull())
  {
    actionBarAdvisor = this->GetWindowAdvisor()->CreateActionBarAdvisor(this->GetWindowConfigurer()->GetActionBarConfigurer());
    poco_assert(actionBarAdvisor.IsNotNull());
  }
  return actionBarAdvisor;
}

Workbench* WorkbenchWindow::GetWorkbenchImpl()
{
  return dynamic_cast<Workbench*>(this->GetWorkbench());
}

void WorkbenchWindow::ShowEmptyWindowContents()
{
  if (!emptyWindowContentsCreated)
  {
    void* parent = this->GetPageComposite();
    emptyWindowContents = this->GetWindowAdvisor()->CreateEmptyWindowContents(
        parent);
    emptyWindowContentsCreated = true;
    //    // force the empty window composite to be layed out
    //    ((StackLayout) parent.getLayout()).topControl = emptyWindowContents;
    //    parent.layout();
  }
}

void WorkbenchWindow::HideEmptyWindowContents()
{
  if (emptyWindowContentsCreated)
  {
    if (emptyWindowContents != 0)
    {
      Tweaklets::Get(GuiWidgetsTweaklet::KEY)->Dispose(emptyWindowContents);
      emptyWindowContents = 0;
      //this->GetPageComposite().layout();
    }
    emptyWindowContentsCreated = false;
  }
}

WorkbenchWindow::ServiceLocatorOwner::ServiceLocatorOwner(WorkbenchWindow* wnd)
: window(wnd)
{

}

void WorkbenchWindow::ServiceLocatorOwner::Dispose()
{
  Shell::Pointer shell = window->GetShell();
  if (shell != 0)
  {
    window->Close();
  }
}

bool WorkbenchWindow::PageList::Add(IWorkbenchPage::Pointer object)
{
  pagesInCreationOrder.push_back(object);
  pagesInActivationOrder.push_front(object);
  // It will be moved to top only when activated.
  return true;
}

WorkbenchWindow::PageList::iterator WorkbenchWindow::PageList::Begin()
{
  return pagesInCreationOrder.begin();
}

WorkbenchWindow::PageList::iterator WorkbenchWindow::PageList::End()
{
  return pagesInCreationOrder.end();
}

bool WorkbenchWindow::PageList::Contains(IWorkbenchPage::Pointer object)
{
  return std::find(pagesInCreationOrder.begin(), pagesInCreationOrder.end(),
      object) != pagesInCreationOrder.end();
}

bool WorkbenchWindow::PageList::Remove(IWorkbenchPage::Pointer object)
{
  if (active == object)
  {
    active = 0;
  }
  pagesInActivationOrder.remove(object);
  unsigned int origSize = pagesInCreationOrder.size();
  pagesInCreationOrder.remove(object);
  return origSize != pagesInCreationOrder.size();
}

void WorkbenchWindow::PageList::Clear()
{
  pagesInCreationOrder.clear();
  pagesInActivationOrder.clear();
  active = 0;
}

bool WorkbenchWindow::PageList::IsEmpty()
{
  return pagesInCreationOrder.empty();
}

const std::list<IWorkbenchPage::Pointer>& WorkbenchWindow::PageList::GetPages()
{
  return pagesInCreationOrder;
}

void WorkbenchWindow::PageList::SetActive(IWorkbenchPage::Pointer page)
{
  if (active == page)
  {
    return;
  }

  active = page;

  if (page.IsNotNull())
  {
    pagesInActivationOrder.remove(page);
    pagesInActivationOrder.push_back(page);
  }
}

WorkbenchPage::Pointer WorkbenchWindow::PageList::GetActive()
{
  return active.Cast<WorkbenchPage>();
}

WorkbenchPage::Pointer WorkbenchWindow::PageList::GetNextActive()
{
  if (active.IsNull())
  {
    if (pagesInActivationOrder.empty())
    {
      return WorkbenchPage::Pointer(0);
    }

    return pagesInActivationOrder.back().Cast<WorkbenchPage>();
  }

  if (pagesInActivationOrder.size() < 2)
  {
    return WorkbenchPage::Pointer(0);
  }

  std::list<IWorkbenchPage::Pointer>::reverse_iterator riter =
  pagesInActivationOrder.rbegin();
  return (++riter)->Cast<WorkbenchPage>();
}

WorkbenchWindow::ShellActivationListener::ShellActivationListener(WorkbenchWindow::Pointer w) :
window(w)
{
}

void WorkbenchWindow::ShellActivationListener::ShellActivated(ShellEvent::Pointer /*event*/)
{
  WorkbenchWindow::Pointer wnd(window);

  wnd->shellActivated = true;
  wnd->serviceLocator->Activate();
  wnd->GetWorkbenchImpl()->SetActivatedWindow(wnd);
  WorkbenchPage::Pointer currentPage = wnd->GetActivePage().Cast<WorkbenchPage>();
  if (currentPage != 0)
  {
    IWorkbenchPart::Pointer part = currentPage->GetActivePart();
    if (part != 0)
    {
      PartSite::Pointer site = part->GetSite().Cast<PartSite>();
      site->GetPane()->ShellActivated();
    }
    IEditorPart::Pointer editor = currentPage->GetActiveEditor();
    if (editor != 0)
    {
      PartSite::Pointer site = editor->GetSite().Cast<PartSite>();
      site->GetPane()->ShellActivated();
    }
    wnd->GetWorkbenchImpl()->FireWindowActivated(wnd);
  }
  //liftRestrictions();
}

void WorkbenchWindow::ShellActivationListener::ShellDeactivated(ShellEvent::Pointer /*event*/)
{
  WorkbenchWindow::Pointer wnd(window);

  wnd->shellActivated = false;
  //imposeRestrictions();
  wnd->serviceLocator->Deactivate();
  WorkbenchPage::Pointer currentPage = wnd->GetActivePage().Cast<WorkbenchPage>();
  if (currentPage != 0)
  {
    IWorkbenchPart::Pointer part = currentPage->GetActivePart();
    if (part != 0)
    {
      PartSite::Pointer site = part->GetSite().Cast<PartSite>();
      site->GetPane()->ShellDeactivated();
    }
    IEditorPart::Pointer editor = currentPage->GetActiveEditor();
    if (editor != 0)
    {
      PartSite::Pointer site = editor->GetSite().Cast<PartSite>();
      site->GetPane()->ShellDeactivated();
    }
    wnd->GetWorkbenchImpl()->FireWindowDeactivated(wnd);
  }
}

void WorkbenchWindow::TrackShellActivation(Shell::Pointer shell)
{
  shellActivationListener = new ShellActivationListener(WorkbenchWindow::Pointer(this));
  shell->AddShellListener(shellActivationListener);
}

WorkbenchWindow::ControlResizeListener::ControlResizeListener(WorkbenchWindow* w)
: window(w)
{
}

GuiTk::IControlListener::Events::Types WorkbenchWindow::ControlResizeListener::GetEventTypes() const
{
  return Events::MOVED & Events::RESIZED;
}

void WorkbenchWindow::
ControlResizeListener::ControlMoved(GuiTk::ControlEvent::Pointer /*e*/)
{
  this->SaveBounds();
}

void WorkbenchWindow::
ControlResizeListener::ControlResized(GuiTk::ControlEvent::Pointer /*e*/)
{
  this->SaveBounds();
}

void WorkbenchWindow::ControlResizeListener::SaveBounds()
{
  WorkbenchWindow::Pointer wnd(window);

  Shell::Pointer shell = wnd->GetShell();
  if (shell == 0)
  {
    return;
  }
  //  if (shell->IsDisposed())
  //  {
  //    return;
  //  }
  if (shell->GetMinimized())
  {
    return;
  }
  if (shell->GetMaximized())
  {
    wnd->asMaximizedState = true;
    return;
  }
  wnd->asMaximizedState = false;
  wnd->normalBounds = shell->GetBounds();
}

void WorkbenchWindow::TrackShellResize(Shell::Pointer newShell)
{
  controlResizeListener = new ControlResizeListener(this);
  Tweaklets::Get(GuiWidgetsTweaklet::KEY)->AddControlListener(newShell->GetControl(), controlResizeListener);
}

}
