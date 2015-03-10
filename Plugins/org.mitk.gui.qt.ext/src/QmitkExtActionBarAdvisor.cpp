/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkExtActionBarAdvisor.h"

#include "QmitkExtActionConstants.h"

#include <berryIActionBarConfigurer.h>
#include <berryIContributionItem.h>
#include <berryMenuManager.h>
#include <berryContributionItemFactory.h>
#include <berryIWorkbenchWindowConfigurer.h>
#include <berryIWorkbenchWindow.h>

QmitkExtActionBarAdvisor::QmitkExtActionBarAdvisor(berry::IActionBarConfigurer::Pointer configurer)
 : berry::ActionBarAdvisor(configurer)
{
  window = configurer->GetWindowConfigurer()->GetWindow().GetPointer();
}

void QmitkExtActionBarAdvisor::FillMenuBar(berry::IMenuManager* menuBar)
{
  //menuBar.add(createFileMenu());
  //menuBar.add(new GroupMarker(IWorkbenchActionConstants.MB_ADDITIONS));
  menuBar->Add(CreateWindowMenu());
  //menuBar.add(createHelpMenu());
}

berry::MenuManager::Pointer QmitkExtActionBarAdvisor::CreateWindowMenu()
{
  berry::MenuManager::Pointer menu(new berry::MenuManager("&Window",
                                                          QmitkExtActionConstants::M_WINDOW));

  //addMacWindowMenuItems(menu);

  //menu.add(newWindowAction);
  //menu.add(newEditorAction);

  //menu.add(new Separator());
  AddPerspectiveActions(menu.GetPointer());
  //menu.add(new Separator());
  //AddKeyboardShortcuts(menu);
  //Separator sep = new Separator(IWorkbenchActionConstants.MB_ADDITIONS);
  //sep.setVisible(!Util.isMac());
  //menu.add(sep);

  //if(Util.isCocoa())
  //  menu.add(arrangeWindowsItem);

  // See the comment for quit in createFileMenu
  //ActionContributionItem openPreferencesItem = new ActionContributionItem(openPreferencesAction);
  //openPreferencesItem.setVisible(!Util.isMac());
  //menu.add(openPreferencesItem);

  //menu.add(ContributionItemFactory.OPEN_WINDOWS.create(getWindow()));
  return menu;
}

void QmitkExtActionBarAdvisor::AddPerspectiveActions(berry::MenuManager* menu)
{
//  {
//    QString openText = IDEWorkbenchMessages.Workbench_openPerspective;
//    MenuManager changePerspMenuMgr = new MenuManager(openText, "openPerspective");
//    IContributionItem changePerspMenuItem = ContributionItemFactory.PERSPECTIVES_SHORTLIST
//        .create(getWindow());
//    changePerspMenuMgr.add(changePerspMenuItem);
//    menu.add(changePerspMenuMgr);
//  }
  {
    berry::MenuManager::Pointer showViewMenuMgr(new berry::MenuManager("Show &View", "showView"));
    berry::IContributionItem::Pointer showViewMenu =
          berry::ContributionItemFactory::VIEWS_SHORTLIST->Create(window);
    showViewMenuMgr->Add(showViewMenu);
    menu->Add(showViewMenuMgr);
  }
  //menu.add(new Separator());
  //menu.add(editActionSetAction);
  //menu.add(savePerspectiveAction);
  //menu.add(resetPerspectiveAction);
  //menu.add(closePerspAction);
  //menu.add(closeAllPerspsAction);
}
