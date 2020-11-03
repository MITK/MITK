/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkExtActionBarAdvisor.h"

#include "QmitkFileOpenAction.h"
#include "QmitkFileSaveAction.h"
#include "QmitkExtFileSaveProjectAction.h"
#include "QmitkCloseProjectAction.h"
#include <QmitkFileExitAction.h>

#include <mitkWorkbenchCommandConstants.h>

#include <berryIActionBarConfigurer.h>
#include <berryIContributionItem.h>
#include <berryMenuManager.h>
#include <berryCommandContributionItem.h>
#include <berryCommandContributionItemParameter.h>
#include <berryContributionItemFactory.h>
#include <berryQActionContributionItem.h>
#include <berryIWorkbenchWindowConfigurer.h>
#include <berryIWorkbenchWindow.h>
#include <berryIWorkbench.h>

#include <berryGroupMarker.h>
#include <berrySeparator.h>
#include <berryWorkbenchActionConstants.h>

QmitkExtActionBarAdvisor::QmitkExtActionBarAdvisor(berry::IActionBarConfigurer::Pointer configurer)
 : berry::ActionBarAdvisor(configurer)
{
  window = configurer->GetWindowConfigurer()->GetWindow().GetPointer();
}

void QmitkExtActionBarAdvisor::MakeActions(berry::IWorkbenchWindow* window)
{
  QAction* fileOpenAction = new QmitkFileOpenAction(
                              QIcon::fromTheme("document-open",
                                               QIcon(":/org_mitk_icons/icons/tango/scalable/actions/document-open.svg")),
                              window);
  fileOpenAction->setShortcut(QKeySequence::Open);
  this->Register(fileOpenAction, berry::IWorkbenchCommandConstants::FILE_OPEN);

  QAction* fileSaveAction = new QmitkFileSaveAction(
                              QIcon(":/org.mitk.gui.qt.ext/Save_48.png"),
                              window);
  fileSaveAction->setShortcut(QKeySequence::Save);
  this->Register(fileSaveAction, berry::IWorkbenchCommandConstants::FILE_SAVE);

  QAction* fileSaveProjectAction = new QmitkExtFileSaveProjectAction(window);
  fileSaveProjectAction->setIcon(QIcon::fromTheme("document-save",
                                                  QIcon(":/org_mitk_icons/icons/tango/scalable/actions/document-save.svg")));
  this->Register(fileSaveProjectAction, mitk::WorkbenchCommandConstants::PROJECT_SAVE);

  QAction* closeProjectAction = new QmitkCloseProjectAction(window);
  closeProjectAction->setIcon(QIcon::fromTheme("edit-delete",
                                               QIcon(":/org_mitk_icons/icons/tango/scalable/actions/edit-delete.svg")));
  this->Register(closeProjectAction, mitk::WorkbenchCommandConstants::PROJECT_CLOSE);

  QAction* fileExitAction = new QmitkFileExitAction(window);
  fileExitAction->setIcon(QIcon::fromTheme("system-log-out", QIcon(":/org_mitk_icons/icons/tango/scalable/actions/system-log-out.svg")));
  fileExitAction->setShortcut(QKeySequence::Quit);
  this->Register(fileExitAction, mitk::WorkbenchCommandConstants::FILE_EXIT);
}

void QmitkExtActionBarAdvisor::FillMenuBar(berry::IMenuManager* menuBar)
{
  menuBar->Add(CreateFileMenu());
  menuBar->Add(CreateEditMenu());
  menuBar->Add(berry::IContributionItem::Pointer(new berry::GroupMarker(berry::WorkbenchActionConstants::MB_ADDITIONS)));
  menuBar->Add(CreateWindowMenu());
  menuBar->Add(CreateHelpMenu());
}

void QmitkExtActionBarAdvisor::FillToolBar(berry::IToolBarManager* /*toolBar*/)
{
}

berry::SmartPointer<berry::MenuManager> QmitkExtActionBarAdvisor::CreateFileMenu()
{
  berry::MenuManager::Pointer menu(new berry::MenuManager("&File",
                                                          berry::WorkbenchActionConstants::M_FILE));

  menu->Add(berry::IContributionItem::Pointer(new berry::GroupMarker(berry::WorkbenchActionConstants::FILE_START)));
  {
    // create the New submenu, using the same id for it as the New action
    QString newText = "&New";
    QString newId = "org.blueberry.ui.newWizard";
    berry::MenuManager::Pointer newMenu(new berry::MenuManager(newText, newId));
    newMenu->SetCommandId("org.blueberry.ui.file.newQuickMenu");
    newMenu->Add(berry::IContributionItem::Pointer(new berry::Separator(newId)));
    //this.newWizardMenu = new NewWizardMenu(getWindow());
    //newMenu.add(this.newWizardMenu);
    newMenu->Add(berry::IContributionItem::Pointer(new berry::Separator(berry::WorkbenchActionConstants::MB_ADDITIONS)));
    menu->Add(newMenu);
  }

  menu->Add(this->GetAction(berry::IWorkbenchCommandConstants::FILE_OPEN),
            berry::IWorkbenchCommandConstants::FILE_OPEN);

  menu->Add(berry::IContributionItem::Pointer(new berry::GroupMarker(berry::WorkbenchActionConstants::NEW_EXT)));
  menu->Add(berry::IContributionItem::Pointer(new berry::Separator()));

  //menu.add(closeAction);
  //menu.add(closeAllAction);
  // // menu.add(closeAllSavedAction);

  menu->Add(this->GetAction(mitk::WorkbenchCommandConstants::PROJECT_CLOSE),
            mitk::WorkbenchCommandConstants::PROJECT_CLOSE);
  menu->Add(berry::IContributionItem::Pointer(new berry::GroupMarker(berry::WorkbenchActionConstants::CLOSE_EXT)));
  menu->Add(berry::IContributionItem::Pointer(new berry::Separator()));

  menu->Add(this->GetAction(berry::IWorkbenchCommandConstants::FILE_SAVE),
            berry::IWorkbenchCommandConstants::FILE_SAVE);
  menu->Add(this->GetAction(mitk::WorkbenchCommandConstants::PROJECT_SAVE),
            mitk::WorkbenchCommandConstants::PROJECT_SAVE);
  menu->Add(berry::IContributionItem::Pointer(new berry::GroupMarker(berry::WorkbenchActionConstants::SAVE_EXT)));
  menu->Add(berry::IContributionItem::Pointer(new berry::Separator()));
  menu->Add(berry::IContributionItem::Pointer(new berry::Separator(berry::WorkbenchActionConstants::MB_ADDITIONS)));

  menu->Add(berry::IContributionItem::Pointer(new berry::Separator()));

  menu->Add(berry::ContributionItemFactory::REOPEN_EDITORS->Create(window));
  menu->Add(berry::IContributionItem::Pointer(new berry::GroupMarker(berry::WorkbenchActionConstants::MRU)));
  menu->Add(berry::IContributionItem::Pointer(new berry::Separator()));

  // If we're on OS X we shouldn't show this command in the File menu. It
  // should be invisible to the user. However, we should not remove it -
  // the carbon UI code will do a search through our menu structure
  // looking for it when Cmd-Q is invoked (or Quit is chosen from the
  // application menu.
  //ActionContributionItem quitItem = new ActionContributionItem(quitAction);
  //quitItem.setVisible(!Util.isMac());
  //menu.add(quitItem);
  menu->Add(this->GetAction(berry::IWorkbenchCommandConstants::FILE_EXIT),
    berry::IWorkbenchCommandConstants::FILE_EXIT);
  menu->Add(berry::IContributionItem::Pointer(new berry::GroupMarker(berry::WorkbenchActionConstants::FILE_END)));

  return menu;
}

berry::SmartPointer<berry::MenuManager> QmitkExtActionBarAdvisor::CreateEditMenu()
{
  berry::MenuManager::Pointer menu(new berry::MenuManager("&Edit",
                                                          berry::WorkbenchActionConstants::M_EDIT));

  menu->Add(berry::IContributionItem::Pointer(new berry::GroupMarker(berry::WorkbenchActionConstants::EDIT_START)));

  menu->Add(GetItem(berry::IWorkbenchCommandConstants::EDIT_UNDO, "&Undo"));
  menu->Add(GetItem(berry::IWorkbenchCommandConstants::EDIT_REDO, "&Redo"));
  menu->Add(berry::IContributionItem::Pointer(new berry::GroupMarker(berry::WorkbenchActionConstants::UNDO_EXT)));
  menu->Add(berry::IContributionItem::Pointer(new berry::Separator()));
/*
  menu.add(getCutItem());
  menu.add(getCopyItem());
  menu.add(getPasteItem());
  menu.add(new GroupMarker(IWorkbenchActionConstants.CUT_EXT));
  menu.add(new Separator());

  menu.add(getDeleteItem());
  menu.add(getSelectAllItem());
  menu.add(new Separator());

  menu.add(getFindItem());
  menu.add(new GroupMarker(IWorkbenchActionConstants.FIND_EXT));
  menu.add(new Separator());

  menu.add(getBookmarkItem());
  menu.add(getTaskItem());
  menu.add(new GroupMarker(IWorkbenchActionConstants.ADD_EXT));
*/
  menu->Add(berry::IContributionItem::Pointer(new berry::GroupMarker(berry::WorkbenchActionConstants::EDIT_END)));
  menu->Add(berry::IContributionItem::Pointer(new berry::Separator(berry::WorkbenchActionConstants::MB_ADDITIONS)));

  return menu;
}

berry::SmartPointer<berry::MenuManager> QmitkExtActionBarAdvisor::CreateHelpMenu()
{
  berry::MenuManager::Pointer menu(new berry::MenuManager("&Help",
                                                          berry::WorkbenchActionConstants::M_HELP));

  menu->Add(berry::IContributionItem::Pointer(new berry::GroupMarker(berry::WorkbenchActionConstants::HELP_START)));

  menu->Add(berry::IContributionItem::Pointer(new berry::Separator("group.intro")));
  // See if a welcome or intro page is specified
  if (window->GetWorkbench()->GetIntroManager()->HasIntro())
  {
    menu->Add(GetItem(berry::IWorkbenchCommandConstants::HELP_WELCOME, "Welcome"));
  }
  menu->Add(berry::IContributionItem::Pointer(new berry::GroupMarker("group.intro.ext")));
  menu->Add(berry::IContributionItem::Pointer(new berry::Separator("group.main")));
  menu->Add(GetItem(berry::IWorkbenchCommandConstants::HELP_HELP_CONTENTS, "Help &Contents"));
  //menu->Add(GetItem(berry::IWorkbenchCommandConstants::HELP_HELP_SEARCH, "Help Search"));
  menu->Add(GetItem(berry::IWorkbenchCommandConstants::HELP_DYNAMIC_HELP, "Conte&xt Help", "", QIcon(), QKeySequence::HelpContents));
  menu->Add(berry::IContributionItem::Pointer(new berry::GroupMarker(berry::WorkbenchActionConstants::HELP_END)));
  menu->Add(berry::IContributionItem::Pointer(new berry::Separator(berry::WorkbenchActionConstants::MB_ADDITIONS)));

  // about should always be at the bottom
  menu->Add(berry::IContributionItem::Pointer(new berry::Separator("group.about")));

  menu->Add(GetItem(berry::IWorkbenchCommandConstants::HELP_ABOUT, "&About"));
  menu->Add(berry::IContributionItem::Pointer(new berry::GroupMarker("group.about.ext")));

  return menu;
}

berry::MenuManager::Pointer QmitkExtActionBarAdvisor::CreateWindowMenu()
{
  berry::MenuManager::Pointer menu(new berry::MenuManager("&Window",
                                                          berry::WorkbenchActionConstants::M_WINDOW));

  // See bug 19059.
  // menu->Add(GetItem(berry::IWorkbenchCommandConstants::WINDOW_NEW_WINDOW, "New Window"));
  // menu->Add(GetItem(berry::IWorkbenchCommandConstants::WINDOW_NEW_EDITOR, "New Editor"));

  menu->Add(berry::IContributionItem::Pointer(new berry::Separator()));
  AddPerspectiveActions(menu.GetPointer());
  //menu.add(new Separator());
  //AddKeyboardShortcuts(menu);
  berry::Separator::Pointer sep(new berry::Separator(berry::WorkbenchActionConstants::MB_ADDITIONS));
  //sep.setVisible(!Util.isMac());
  menu->Add(berry::IContributionItem::Pointer(sep.GetPointer()));

  //if(Util.isCocoa())
  //  menu.add(arrangeWindowsItem);

  // See the comment for quit in createFileMenu
  berry::IContributionItem::Pointer openPreferencesItem =
      GetItem(berry::IWorkbenchCommandConstants::WINDOW_PREFERENCES, "Preferences");
  //openPreferencesItem.setVisible(!Util.isMac());
  menu->Add(openPreferencesItem);

  menu->Add(berry::ContributionItemFactory::OPEN_WINDOWS->Create(window));
  return menu;
}

void QmitkExtActionBarAdvisor::AddPerspectiveActions(berry::MenuManager* menu)
{
  {
    berry::MenuManager::Pointer changePerspMenuMgr(new berry::MenuManager("Open Perspective", "openPerspective"));
    berry::IContributionItem::Pointer changePerspMenuItem =
        berry::ContributionItemFactory::PERSPECTIVES_SHORTLIST->Create(window);
    changePerspMenuMgr->Add(changePerspMenuItem);
    menu->Add(changePerspMenuMgr);
  }
  {
    berry::MenuManager::Pointer showViewMenuMgr(new berry::MenuManager("Show &View", "showView"));
    berry::IContributionItem::Pointer showViewMenu =
          berry::ContributionItemFactory::VIEWS_SHORTLIST->Create(window);
    showViewMenuMgr->Add(showViewMenu);
    menu->Add(showViewMenuMgr);
  }
  menu->Add(berry::IContributionItem::Pointer(new berry::Separator()));
  //menu.add(editActionSetAction);
  menu->Add(GetItem(berry::IWorkbenchCommandConstants::WINDOW_SAVE_PERSPECTIVE_AS, "Save Perspective &As..."));
  menu->Add(GetItem(berry::IWorkbenchCommandConstants::WINDOW_RESET_PERSPECTIVE, "&Reset Perspective..."));
  menu->Add(GetItem(berry::IWorkbenchCommandConstants::WINDOW_CLOSE_PERSPECTIVE, "&Close Perspective"));
  menu->Add(GetItem(berry::IWorkbenchCommandConstants::WINDOW_CLOSE_ALL_PERSPECTIVES, "Close All Perspectives"));
}

berry::SmartPointer<berry::IContributionItem> QmitkExtActionBarAdvisor::GetItem(const QString& commandId, const QString& label,
                                                                                const QString& tooltip, const QIcon& icon, const QKeySequence& shortcut)
{
  berry::CommandContributionItemParameter::Pointer param(
        new berry::CommandContributionItemParameter(window, QString(), commandId, berry::CommandContributionItem::STYLE_PUSH));
  param->icon = icon;
  param->label = label;
  param->tooltip = tooltip;
  param->shortcut = shortcut;
  berry::IContributionItem::Pointer item(new berry::CommandContributionItem(param));
  return item;
}
