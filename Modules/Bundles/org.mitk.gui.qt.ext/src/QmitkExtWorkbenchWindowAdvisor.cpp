/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include "QmitkExtWorkbenchWindowAdvisor.h"
#include "QmitkExtActionBarAdvisor.h"


#include <QMenu>
#include <QMenuBar>
#include <QMainWindow>
#include <QStatusBar>

#include <cherryPlatform.h>
#include <cherryPlatformUI.h>
#include <cherryIWorkbenchWindow.h>
#include <cherryIWorkbenchPage.h>
#include <cherryIPreferencesService.h>
#include <cherryIPerspectiveRegistry.h>
#include <cherryIPerspectiveDescriptor.h>

#include <internal/cherryQtShowViewAction.h>
#include <internal/cherryQtOpenPerspectiveAction.h>

#include <QmitkFileOpenAction.h>
#include <QmitkFileSaveProjectAction.h>
#include <QmitkFileExitAction.h>
#include <QmitkStatusBar.h>
#include <QmitkProgressBar.h>
#include <QmitkMemoryUsageIndicatorView.h>
#include <QmitkPreferencesDialog.h>


// UGLYYY
#include "internal/QmitkExtWorkbenchWindowAdvisorHack.h"
#include "mitkUndoController.h"
#include "mitkVerboseLimitedLinearUndo.h"
#include <QToolBar>
#include <QMessageBox>

QmitkExtWorkbenchWindowAdvisorHack* QmitkExtWorkbenchWindowAdvisorHack::undohack = new QmitkExtWorkbenchWindowAdvisorHack();

QmitkExtWorkbenchWindowAdvisor::QmitkExtWorkbenchWindowAdvisor(cherry::IWorkbenchWindowConfigurer::Pointer configurer)
: cherry::WorkbenchWindowAdvisor(configurer),
showViewToolbar(true)
{
  configurer->SetShowPerspectiveBar(true);
}

cherry::ActionBarAdvisor::Pointer
QmitkExtWorkbenchWindowAdvisor::CreateActionBarAdvisor(
  cherry::IActionBarConfigurer::Pointer configurer)
{
  cherry::ActionBarAdvisor::Pointer actionBarAdvisor(new QmitkExtActionBarAdvisor(configurer));
  return actionBarAdvisor;
}

void QmitkExtWorkbenchWindowAdvisor::ShowViewToolbar(bool show)
{
  showViewToolbar = show;
}

void QmitkExtWorkbenchWindowAdvisor::PostWindowCreate()
{
  // very bad hack...
  cherry::IWorkbenchWindow::Pointer window = this->GetWindowConfigurer()->GetWindow();
  QMainWindow* mainWindow = static_cast<QMainWindow*>(window->GetShell()->GetControl());


  // ==== Application menu ============================
  QMenuBar* menuBar = mainWindow->menuBar();

  QMenu* fileMenu = menuBar->addMenu("&File");

  fileMenu->addAction(new QmitkFileOpenAction(window));
  fileMenu->addAction(new QmitkFileSaveProjectAction(window));
  fileMenu->addSeparator();
  fileMenu->addAction(new QmitkFileExitAction(window));

  cherry::IViewRegistry* viewRegistry = cherry::PlatformUI::GetWorkbench()->GetViewRegistry();
  const std::vector<cherry::IViewDescriptor::Pointer>& viewDescriptors = viewRegistry->GetViews();

  // another bad hack to get an edit/undo menu... 
  QMenu* editMenu = menuBar->addMenu("&Edit");
  QAction* undoAction = editMenu->addAction("&Undo", QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onUndo()), QKeySequence("CTRL+Z"));
  QAction* redoAction = editMenu->addAction("&Redo", QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onRedo()), QKeySequence("CTRL+Y"));

  // ==== Window Menu ==========================
  QMenu* windowMenu = menuBar->addMenu("Window");
  QAction* newWindowAction = windowMenu->addAction("&New Window", QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onNewWindow()));
  windowMenu->addSeparator();
  QMenu* perspMenu = windowMenu->addMenu("&Open Perspective");
  QMenu* viewMenu = windowMenu->addMenu("Show &View");
  windowMenu->addSeparator();
  QAction* resetPerspectiveAction = windowMenu->addAction("&Reset Perspective", QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onResetPerspective()));
  QAction* closePerspectiveAction = windowMenu->addAction("&Close Perspective", QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onClosePerspective()));
  windowMenu->addSeparator();
  QAction* preferencesAction = windowMenu->addAction("&Preferences...", QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onEditPreferences()), QKeySequence("CTRL+P"));

  // fill perspective menu
  cherry::IPerspectiveRegistry* perspRegistry = window->GetWorkbench()->GetPerspectiveRegistry();
  QActionGroup* perspGroup = new QActionGroup(menuBar);

  std::vector<cherry::IPerspectiveDescriptor::Pointer> perspectives(perspRegistry->GetPerspectives());
  for (std::vector<cherry::IPerspectiveDescriptor::Pointer>::iterator perspIt =
      perspectives.begin(); perspIt != perspectives.end(); ++perspIt)
  {
    QAction* perspAction = new cherry::QtOpenPerspectiveAction(window, *perspIt, perspGroup);
  }
  perspMenu->addActions(perspGroup->actions());

  // sort elements (converting vector to map...)
  std::vector<cherry::IViewDescriptor::Pointer>::const_iterator iter;
  std::map<std::string, cherry::IViewDescriptor::Pointer> VDMap;

  for (iter = viewDescriptors.begin(); iter != viewDescriptors.end(); ++iter)
  {
    if ((*iter)->GetId() == "org.opencherry.ui.internal.introview") continue;
    std::pair<std::string, cherry::IViewDescriptor::Pointer> p((*iter)->GetLabel(), (*iter)); 
    VDMap.insert(p);
  }
  // ==================================================

  // ==== View Toolbar ==================================
  QToolBar* qToolbar = new QToolBar;
  
  std::map<std::string, cherry::IViewDescriptor::Pointer>::const_iterator MapIter;
  for (MapIter = VDMap.begin(); MapIter != VDMap.end(); ++MapIter)
  {
    cherry::QtShowViewAction* viewAction = new cherry::QtShowViewAction(window, (*MapIter).second);
    //m_ViewActions.push_back(viewAction);
    viewMenu->addAction(viewAction);
    if (showViewToolbar)
    {
      qToolbar->addAction(viewAction);
    }
  }
  
  if (showViewToolbar)
  {
    mainWindow->addToolBar(qToolbar);
  }
  else delete qToolbar;
  // ====================================================

  // ===== Help menu ====================================
  QMenu* helpMenu = menuBar->addMenu("Help");
  QAction* welcomeAction = helpMenu->addAction("&Welcome", QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onIntro()));
  QAction* aboutAction = helpMenu->addAction("&About",QmitkExtWorkbenchWindowAdvisorHack::undohack, SLOT(onIntro()));
  // =====================================================


  QStatusBar* qStatusBar = new QStatusBar();

  //creating a QmitkStatusBar for Output on the QStatusBar and connecting it with the MainStatusBar
  QmitkStatusBar *statusBar = new QmitkStatusBar(qStatusBar);
  //disabling the SizeGrip in the lower right corner
  statusBar->SetSizeGripEnabled(false);

  QmitkProgressBar *progBar = new QmitkProgressBar();
  qStatusBar->addPermanentWidget(progBar, 0);
  progBar->hide();
  mainWindow->setStatusBar(qStatusBar);

  QmitkMemoryUsageIndicatorView* memoryIndicator = new QmitkMemoryUsageIndicatorView();
  qStatusBar->addPermanentWidget(memoryIndicator, 0);
}
    
//--------------------------------------------------------------------------------
// Ugly hack from here on. Feel free to delete when command framework 
// and undo buttons are done.
//--------------------------------------------------------------------------------

QmitkExtWorkbenchWindowAdvisorHack::QmitkExtWorkbenchWindowAdvisorHack()
:QObject()
{
}
    
void QmitkExtWorkbenchWindowAdvisorHack::onUndo()
{
  mitk::UndoModel* model = mitk::UndoController::GetCurrentUndoModel();
  if (model)
  {
    if (mitk::VerboseLimitedLinearUndo* verboseundo = dynamic_cast<mitk::VerboseLimitedLinearUndo*>( model ))
    {
      mitk::VerboseLimitedLinearUndo::StackDescription descriptions = verboseundo->GetUndoDescriptions();
      if (descriptions.size() >= 1)
      {
        LOG_INFO << "Undo " << descriptions.front().second;
      }
    }
    model->Undo();
  }
  else
  {
    LOG_ERROR << "No undo model instantiated";
  }
}

void QmitkExtWorkbenchWindowAdvisorHack::onRedo()
{
  mitk::UndoModel* model = mitk::UndoController::GetCurrentUndoModel();
  if (model)
  {
    if (mitk::VerboseLimitedLinearUndo* verboseundo = dynamic_cast<mitk::VerboseLimitedLinearUndo*>( model ))
    {
      mitk::VerboseLimitedLinearUndo::StackDescription descriptions = verboseundo->GetRedoDescriptions();
      if (descriptions.size() >= 1)
      {
        LOG_INFO << "Redo " << descriptions.front().second;
      }
    }
    model->Redo();
  }
  else
  {
    LOG_ERROR << "No undo model instantiated";
  }
}

void QmitkExtWorkbenchWindowAdvisorHack::onEditPreferences()
{
  QmitkPreferencesDialog _PreferencesDialog(QApplication::activeWindow());
  _PreferencesDialog.exec();
}

void QmitkExtWorkbenchWindowAdvisorHack::onQuit()
{
  cherry::PlatformUI::GetWorkbench()->Close();
}

void QmitkExtWorkbenchWindowAdvisorHack::onResetPerspective()
{
  cherry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage()->ResetPerspective();
}

void QmitkExtWorkbenchWindowAdvisorHack::onClosePerspective()
{
  cherry::IWorkbenchPage::Pointer page = cherry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage();
  page->ClosePerspective(page->GetPerspective(), true, true);
}

void QmitkExtWorkbenchWindowAdvisorHack::onNewWindow()
{
  cherry::PlatformUI::GetWorkbench()->OpenWorkbenchWindow(0);
}

void QmitkExtWorkbenchWindowAdvisorHack::onIntro()
{
  bool hasIntro = cherry::PlatformUI::GetWorkbench()->GetIntroManager()->HasIntro();
  if (!hasIntro)
  {
    QMessageBox::information(0, "No Welcome Content Found", "There is no welcome content suitable for display in this application.");
  }
  else
  {
    cherry::PlatformUI::GetWorkbench()->GetIntroManager()->ShowIntro(
       cherry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow(), false);
  }
}
