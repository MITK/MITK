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

#include "cherryIWorkbenchPage.h"
#include "cherryWorkbench.h"

#include "cherryPlatformUI.h"

namespace cherry
{

const int WorkbenchWindow::FILL_ALL_ACTION_BARS = ActionBarAdvisor::FILL_MENU_BAR
      | ActionBarAdvisor::FILL_COOL_BAR
      | ActionBarAdvisor::FILL_STATUS_LINE;

WorkbenchWindow::WorkbenchWindow()
 : windowAdvisor(0), actionBarAdvisor(0)
 {
  // Make sure there is a workbench. This call will throw
  // an exception if workbench not created yet.
  IWorkbench::Pointer workbench = PlatformUI::GetWorkbench();
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

void WorkbenchWindow::FireWindowOpening() 
{
  // let the application do further configuration
  this->GetWindowAdvisor()->PreWindowOpen();
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

IWorkbenchPage::Pointer WorkbenchWindow::GetActivePage()
{
  // TODO: return the active WorkbenchPage
  return SmartPointer<IWorkbenchPage>();
}

IWorkbench::Pointer WorkbenchWindow::GetWorkbench()
{
  return PlatformUI::GetWorkbench();
}


IPartService* WorkbenchWindow::GetPartService()
{
  return 0;
}

ISelectionService* WorkbenchWindow::GetSelectionService()
{
  return 0;
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

WorkbenchWindowConfigurer::Pointer WorkbenchWindow::GetWindowConfigurer() {
    if (windowConfigurer.IsNull()) {
      // lazy initialize
      windowConfigurer = new WorkbenchWindowConfigurer(this);
    }
    return windowConfigurer;
  }

WorkbenchAdvisor* WorkbenchWindow::GetAdvisor() {
    return this->GetWorkbenchImpl()->GetAdvisor();
  }

WorkbenchWindowAdvisor* WorkbenchWindow::GetWindowAdvisor() {
  if (windowAdvisor == 0) {
    windowAdvisor = this->GetAdvisor()->CreateWorkbenchWindowAdvisor(
        this->GetWindowConfigurer());
    poco_check_ptr(windowAdvisor);
  }
  return windowAdvisor;
}

ActionBarAdvisor::Pointer WorkbenchWindow::GetActionBarAdvisor() {
  if (actionBarAdvisor.IsNull()) {
    actionBarAdvisor = this->GetWindowAdvisor()->CreateActionBarAdvisor(
        this->GetWindowConfigurer()->GetActionBarConfigurer());
    poco_assert(actionBarAdvisor.IsNotNull());
  }
  return actionBarAdvisor;
}

Workbench::Pointer WorkbenchWindow::GetWorkbenchImpl() {
  return this->GetWorkbench().Cast<Workbench>();
}

}
