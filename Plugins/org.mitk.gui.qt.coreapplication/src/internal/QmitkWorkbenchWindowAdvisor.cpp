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

#include "QmitkWorkbenchWindowAdvisor.h"
#include "QmitkActionBarAdvisor.h"


#include <QMenu>
#include <QMenuBar>
#include <QMainWindow>
#include <QStatusBar>

#include <berryPlatform.h>
#include <berryPlatformUI.h>
#include <berryIActionBarConfigurer.h>
#include <berryIWorkbenchWindow.h>
#include <berryIPreferencesService.h>

#include <internal/berryQtShowViewAction.h>

#include <QmitkFileOpenAction.h>
#include <QmitkFileExitAction.h>
#include <QmitkStatusBar.h>
#include <QmitkProgressBar.h>
#include <QmitkMemoryUsageIndicatorView.h>
#include <QmitkDefaultDropTargetListener.h>

#include <QToolBar>

QmitkWorkbenchWindowAdvisor::QmitkWorkbenchWindowAdvisor(berry::IWorkbenchWindowConfigurer::Pointer configurer)
: berry::WorkbenchWindowAdvisor(configurer)
, dropTargetListener(new QmitkDefaultDropTargetListener)
{
}

berry::ActionBarAdvisor::Pointer
QmitkWorkbenchWindowAdvisor::CreateActionBarAdvisor(
  berry::IActionBarConfigurer::Pointer configurer)
{
  berry::ActionBarAdvisor::Pointer actionBarAdvisor(new QmitkActionBarAdvisor(configurer));
  return actionBarAdvisor;
}

void QmitkWorkbenchWindowAdvisor::PostWindowCreate()
{
  // very bad hack...
  berry::IWorkbenchWindow::Pointer window = this->GetWindowConfigurer()->GetWindow();
  QMainWindow* mainWindow = static_cast<QMainWindow*>(window->GetShell()->GetControl());

  QMenuBar* menuBar = mainWindow->menuBar();

  QMenu* fileMenu = menuBar->addMenu("&File");

  fileMenu->addAction(new QmitkFileOpenAction(window));
  fileMenu->addSeparator();
  fileMenu->addAction(new QmitkFileExitAction(window));

  berry::IViewRegistry* viewRegistry = berry::PlatformUI::GetWorkbench()->GetViewRegistry();
  QList<berry::IViewDescriptor::Pointer> viewDescriptors = viewRegistry->GetViews();

  QMenu* viewMenu = menuBar->addMenu("Show &View");

  // sort elements (converting vector to map...)
  std::map<QString, berry::IViewDescriptor::Pointer> VDMap;

  for (auto iter = viewDescriptors.begin(); iter != viewDescriptors.end(); ++iter)
  {
    if ((*iter)->GetId() == "org.blueberry.ui.internal.introview")
      continue;
    std::pair<QString, berry::IViewDescriptor::Pointer> p((*iter)->GetLabel(), (*iter));
    VDMap.insert(p);
  }

  QToolBar* qToolbar = new QToolBar;

  for (auto MapIter = VDMap.begin(); MapIter != VDMap.end(); ++MapIter)
  {
    berry::QtShowViewAction* viewAction = new berry::QtShowViewAction(window, (*MapIter).second);
    //m_ViewActions.push_back(viewAction);
    viewMenu->addAction(viewAction);
    qToolbar->addAction(viewAction);
  }

  mainWindow->addToolBar(qToolbar);

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

void QmitkWorkbenchWindowAdvisor::PreWindowOpen()
{
  this->GetWindowConfigurer()->AddEditorAreaTransfer(QStringList("text/uri-list"));
  this->GetWindowConfigurer()->ConfigureEditorAreaDropListener(dropTargetListener.data());
}
