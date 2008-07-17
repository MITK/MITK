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

#include "cherryWorkbenchWindow.h"
#include "cherryIPerspectiveDescriptor.h"
#include "cherryUIException.h"

#include "internal/cherryWorkbenchPlugin.h"
#include "internal/cherryWorkbenchPage.h"
#include "internal/cherryWorkbench.h"

#include "cherryPlatformUI.h"

namespace cherry
{

const int WorkbenchWindow::FILL_ALL_ACTION_BARS =
    ActionBarAdvisor::FILL_MENU_BAR | ActionBarAdvisor::FILL_COOL_BAR
        | ActionBarAdvisor::FILL_STATUS_LINE;

WorkbenchWindow::WorkbenchWindow(int number) :
  windowAdvisor(0), actionBarAdvisor(0), windowManager(0), closing(false), updateDisabled(true)
{
  this->number = number;
  // Make sure there is a workbench. This call will throw
  // an exception if workbench not created yet.
  PlatformUI::GetWorkbench();
  //  IServiceLocatorCreator slc = (IServiceLocatorCreator) workbench
  //      .getService(IServiceLocatorCreator.class);
  //  this.serviceLocator = (ServiceLocator) slc
  //      .createServiceLocator(workbench, null);
  //  initializeDefaultServices();
}

void WorkbenchWindow::Init()
{
  // Add contribution managers that are exposed to other plugins.
  //addMenuBar();
  //addCoolBar(SWT.NONE);  // style is unused
  //addStatusLine();

  this->FireWindowOpening();

  // Fill the action bars
  this->FillActionBars(FILL_ALL_ACTION_BARS);
}

void WorkbenchWindow::FireWindowOpening() {
  // let the application do further configuration
  this->GetWindowAdvisor()->PreWindowOpen();
}

void WorkbenchWindow::FireWindowRestored() {
  //StartupThreading.runWithWorkbenchExceptions(new StartupRunnable() {
  //  public void runWithException() throws Throwable {
      this->GetWindowAdvisor()->PostWindowRestore();
  //  }
  //});
}

void WorkbenchWindow::FireWindowCreated() {
  this->GetWindowAdvisor()->PostWindowCreate();
}

void WorkbenchWindow::FireWindowOpened() {
  this->GetWorkbenchImpl()->FireWindowOpened(this);
  this->GetWindowAdvisor()->PostWindowOpen();
}

bool WorkbenchWindow::FireWindowShellClosing() {
  return this->GetWindowAdvisor()->PreWindowShellClose();
}

void WorkbenchWindow::FireWindowClosed() {
  // let the application do further deconfiguration
  this->GetWindowAdvisor()->PostWindowClose();
  this->GetWorkbenchImpl()->FireWindowClosed(this);
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
    // also check for starting - if the first window dies on startup
    // then we'll need to open a default window.
    if (!workbench->IsStarting()
        && !workbench->IsClosing()
        && count <= 1
        && workbench->GetWorkbenchConfigurer()
        ->GetExitOnLastWindowClose())
    {
      windowClosed = workbench->Close();
    }
    else
    {
      if (this->OkToClose())
      {
        windowClosed = this->HardClose();
      }
    }
  }
  catch (std::exception& exc)
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

bool WorkbenchWindow::OkToClose() {
  // Save all of the editors.
  if (!this->GetWorkbenchImpl()->IsClosing()) {
    if (!this->SaveAllPages(true)) {
      return false;
    }
  }
  return true;
}

bool WorkbenchWindow::SaveAllPages(bool bConfirm) {
    bool bRet = true;
    PageList::iterator itr = pageList.Begin();
    while (bRet && itr != pageList.End()) {
      bRet = (*itr)->SaveAllEditors(bConfirm);
      ++itr;
    }
    return bRet;
  }

bool WorkbenchWindow::HardClose()
{
  bool result;
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
  }
  catch (std::exception& exc)
  {
    // Bring down all of the services ... after the window goes away
    //serviceLocator.dispose();
    //menuRestrictions.clear();
    throw exc;
  }
  return result;
}

void WorkbenchWindow::CloseAllPages() {
    // Deactivate active page.
    this->SetActivePage(0);

    // Clone and deref all so that calls to getPages() returns
    // empty list (if called by pageClosed event handlers)
    PageList oldList = pageList;
    pageList.Clear();

    // Close all.
    for (PageList::iterator itr = oldList.Begin(); itr != oldList.End(); ++itr) {
      //(*itr)->FirePageClosed(page);
      //page.dispose();
    }
    if (!closing) {
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
  return this->GetActivePage();
}

ISelectionService* WorkbenchWindow::GetSelectionService()
{
  return this->GetActivePage();
}

bool WorkbenchWindow::IsClosing()
{
  return closing || this->GetWorkbenchImpl()->IsClosing();
}

int WorkbenchWindow::Open() {
//  if (getPages().length == 0) {
//    showEmptyWindowContents();
//  }
  this->FireWindowCreated();
  this->GetWindowAdvisor()->OpenIntro();

  int result = this->OpenImpl();

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

bool WorkbenchWindow::RestoreState(IMemento::Pointer memento,
    IPerspectiveDescriptor::Pointer activeDescriptor) {
//  Assert.isNotNull(getShell());
//
  bool result = true;
//  final MultiStatus result = new MultiStatus(PlatformUI.PLUGIN_ID, IStatus.OK,
//      WorkbenchMessages.WorkbenchWindow_problemsRestoringWindow, null);
//
//  // Restore the window advisor state.
//  IMemento windowAdvisorState = memento
//      .getChild(IWorkbenchConstants.TAG_WORKBENCH_WINDOW_ADVISOR);
//  if (windowAdvisorState != null) {
//    result.add(getWindowAdvisor().restoreState(windowAdvisorState));
//  }
//
//  // Restore actionbar advisor state.
//  IMemento actionBarAdvisorState = memento
//      .getChild(IWorkbenchConstants.TAG_ACTION_BAR_ADVISOR);
//  if (actionBarAdvisorState != null) {
//    result.add(getActionBarAdvisor()
//        .restoreState(actionBarAdvisorState));
//  }
//
//  // Read window's bounds and state.
//  final Rectangle [] displayBounds = new Rectangle[1];
//  StartupThreading.runWithoutExceptions(new StartupRunnable() {
//
//    public void runWithException() {
//      displayBounds[0] = getShell().getDisplay().getBounds();
//
//    }});
//  final Rectangle shellBounds = new Rectangle(0, 0, 0, 0);
//
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
//  Integer bigInt = memento.getInteger(IWorkbenchConstants.TAG_X);
//  shellBounds.x = bigInt == null ? 0 : bigInt.intValue();
//  bigInt = memento.getInteger(IWorkbenchConstants.TAG_Y);
//  shellBounds.y = bigInt == null ? 0 : bigInt.intValue();
//  bigInt = memento.getInteger(IWorkbenchConstants.TAG_WIDTH);
//  shellBounds.width = bigInt == null ? 0 : bigInt.intValue();
//  bigInt = memento.getInteger(IWorkbenchConstants.TAG_HEIGHT);
//  shellBounds.height = bigInt == null ? 0 : bigInt.intValue();
//  if (!shellBounds.isEmpty()) {
//    StartupThreading.runWithoutExceptions(new StartupRunnable() {
//
//      public void runWithException() {
//        if (!shellBounds.intersects(displayBounds[0])) {
//          Rectangle clientArea = getShell().getDisplay().getClientArea();
//          shellBounds.x = clientArea.x;
//          shellBounds.y = clientArea.y;
//        }
//        getShell().setBounds(shellBounds);
//      }});
//  }
//  if ("true".equals(memento.getString(IWorkbenchConstants.TAG_MAXIMIZED))) { //$NON-NLS-1$
//    StartupThreading.runWithoutExceptions(new StartupRunnable() {
//
//      public void runWithException() {
//        getShell().setMaximized(true);
//      }});
//
//  }
//  if ("true".equals(memento.getString(IWorkbenchConstants.TAG_MINIMIZED))) { //$NON-NLS-1$
//    // getShell().setMinimized(true);
//  }
//
//  // restore the width of the perspective bar
//  if (perspectiveSwitcher != null) {
//    perspectiveSwitcher.restoreState(memento);
//  }
//
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
//
//  // Recreate each page in the window.
    IWorkbenchPage::Pointer newActivePage;
//  IMemento[] pageArray = memento
//      .getChildren(IWorkbenchConstants.TAG_PAGE);
//  for (int i = 0; i < pageArray.length; i++) {
//    final IMemento pageMem = pageArray[i];
//    String strFocus = pageMem.getString(IWorkbenchConstants.TAG_FOCUS);
//    if (strFocus == null || strFocus.length() == 0) {
//      continue;
//    }
//
//    // Get the input factory.
//    final IAdaptable [] input = new IAdaptable[1];
//    final IMemento inputMem = pageMem.getChild(IWorkbenchConstants.TAG_INPUT);
//    if (inputMem != null) {
//      final String factoryID = inputMem
//          .getString(IWorkbenchConstants.TAG_FACTORY_ID);
//      if (factoryID == null) {
//        WorkbenchPlugin
//            .log("Unable to restore page - no input factory ID."); //$NON-NLS-1$
//        result.add(unableToRestorePage(pageMem));
//        continue;
//      }
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
//    }
//    // Open the perspective.
//    final IAdaptable finalInput = input[0];
//    final WorkbenchPage [] newPage = new WorkbenchPage[1];
//    try {
//      StartupThreading.runWithWorkbenchExceptions(new StartupRunnable(){
//
//        public void runWithException() throws WorkbenchException {
//          newPage[0] = ((WorkbenchImplementation) Tweaklets
//              .get(WorkbenchImplementation.KEY)).createWorkbenchPage(WorkbenchWindow.this, finalInput);
//        }});
//
//      result.add(newPage[0].restoreState(pageMem, activeDescriptor));
//      pageList.add(newPage[0]);
//      StartupThreading.runWithoutExceptions(new StartupRunnable() {
//
//        public void runWithException() throws Throwable {
//          firePageOpened(newPage[0]);
//        }});
//
//    } catch (WorkbenchException e) {
//      WorkbenchPlugin
//          .log(
//              "Unable to restore perspective - constructor failed.", e); //$NON-NLS-1$
//      result.add(e.getStatus());
//      continue;
//    }
//
//    if (strFocus != null && strFocus.length() > 0) {
//      newActivePage = newPage[0];
//    }
//  }

  // If there are no pages create a default.
  if (pageList.IsEmpty()) {
    try {
      //TODO perspective
//      final String defPerspID = getWorkbenchImpl().getPerspectiveRegistry()
//          .getDefaultPerspective();
      std::string defPerspID = "";
      //if (defPerspID != null) {
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
        //  }});
      //}
    } catch (WorkbenchException& e) {
      WorkbenchPlugin
          ::Log(
              "Unable to create default perspective - constructor failed.", e); //$NON-NLS-1$
      result = false;
//      String productName = WorkbenchPlugin.getDefault()
//          .getProductName();
//      if (productName == null) {
//        productName = ""; //$NON-NLS-1$
//      }
//      getShell().setText(productName);
    }
  }

  // Set active page.
  if (newActivePage.IsNull()) {
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
  //TODO perspective
  //std::string perspId = this->GetWorkbenchImpl()->GetPerspectiveRegistry()
  //->GetDefaultPerspective();
  return this->OpenPage("", input);
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
    this->SetActivePage(newPage);
  }
  else
  {
    IWorkbenchWindow::Pointer window = this->GetWorkbench()->OpenWorkbenchWindow(perspID, input);
    newPage = window->GetActivePage();
  }

  return newPage;
}

void WorkbenchWindow::SetWindowManager(WindowManager* manager) {
  windowManager = manager;

  // Code to detect invalid usage

  if (manager != 0) {
    std::vector<IWorkbenchWindow::Pointer> windows = manager->GetWindows();
    if (std::find(windows.begin(), windows.end(), this) != windows.end())
      return;

    manager->Add(this);
  }
}

WindowManager* WorkbenchWindow::GetWindowManager() {
  return windowManager;
}

int WorkbenchWindow::GetNumber()
{
  return number;
}

/**
 * Sets the active page within the window.
 *
 * @param in
 *            identifies the new active page, or <code>null</code> for no
 *            active page
 */
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
    //hideEmptyWindowContents();
    newPage->OnActivate();
    //this->FirePageActivated(newPage);
    //TODO perspective
//    if (newPage->GetPerspective().IsNotNull())
//    {
//      this->FirePerspectiveActivated(newPage, newPage
//      ->GetPerspective());
//    }
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

  //  IWorkbenchPage activePage = getActivePage();
  //  if (activePage != null
  //      && activePage.findView(IIntroConstants.INTRO_VIEW_ID) != null) {
  //    IMemento introMem = memento
  //        .createChild(IWorkbenchConstants.TAG_INTRO);
  //    boolean isStandby = getWorkbench()
  //        .getIntroManager()
  //        .isIntroStandby(getWorkbench().getIntroManager().getIntro());
  //    introMem.putString(IWorkbenchConstants.TAG_STANDBY, String
  //        .valueOf(isStandby));
  //  }
  //
  //  // save the width of the perspective bar
  //  IMemento persBarMem = memento
  //      .createChild(IWorkbenchConstants.TAG_PERSPECTIVE_BAR);
  //  if (perspectiveSwitcher != null) {
  //    perspectiveSwitcher.saveState(persBarMem);
  //  }
  //
  //
  //  // Save each page.
  //  Iterator itr = pageList.iterator();
  //  while (itr.hasNext()) {
  //    WorkbenchPage page = (WorkbenchPage) itr.next();
  //
  //    // Save perspective.
  //    IMemento pageMem = memento
  //        .createChild(IWorkbenchConstants.TAG_PAGE);
  //    pageMem.putString(IWorkbenchConstants.TAG_LABEL, page.getLabel());
  //    result.add(page.saveState(pageMem));
  //
  //    if (page == getActiveWorkbenchPage()) {
  //      pageMem.putString(IWorkbenchConstants.TAG_FOCUS, "true"); //$NON-NLS-1$
  //    }
  //
  //    // Get the input.
  //    IAdaptable input = page.getInput();
  //    if (input != null) {
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
  //  }
  //
  //  // Save window advisor state.
  //  IMemento windowAdvisorState = memento
  //      .createChild(IWorkbenchConstants.TAG_WORKBENCH_WINDOW_ADVISOR);
  //  result.add(getWindowAdvisor().saveState(windowAdvisorState));
  //
  //  // Save actionbar advisor state.
  //  IMemento actionBarAdvisorState = memento
  //      .createChild(IWorkbenchConstants.TAG_ACTION_BAR_ADVISOR);
  //  result.add(getActionBarAdvisor().saveState(actionBarAdvisorState));
  //
  //  // Only save the trim state if we're using the default layout
  //  if (defaultLayout != null) {
  //    IMemento trimState = memento.createChild(IWorkbenchConstants.TAG_TRIM);
  //    result.add(saveTrimState(trimState));
  //  }
  //
  //  return result;
  return true;
}

WorkbenchWindowConfigurer::Pointer WorkbenchWindow::GetWindowConfigurer()
{
  if (windowConfigurer.IsNull())
  {
    // lazy initialize
    windowConfigurer = new WorkbenchWindowConfigurer(this);
  }
  return windowConfigurer;
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
      return 0;
    }

    return pagesInActivationOrder.back().Cast<WorkbenchPage>();
  }

  if (pagesInActivationOrder.size() < 2)
  {
    return 0;
  }

  std::list<IWorkbenchPage::Pointer>::reverse_iterator riter =
      pagesInActivationOrder.rbegin();
  return (++riter)->Cast<WorkbenchPage>();
}

}
