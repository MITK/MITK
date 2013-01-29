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

#include "CustomViewerWorkbenchWindowAdvisor.h"
#include "org_mitk_example_gui_customviewer_Activator.h"
#include "QtPerspectiveSwitcherTabBar.h"

#include <QHBoxLayout>
#include <QMainWindow>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>
#include <QFileDialog>

#include <mitkCoreObjectFactory.h>
#include <mitkWorkbenchUtil.h>

#include <berryIQtStyleManager.h>
#include <berryIWorkbench.h>


CustomViewerWorkbenchWindowAdvisor::CustomViewerWorkbenchWindowAdvisor(berry::IWorkbenchWindowConfigurer::Pointer configurer)
: berry::WorkbenchWindowAdvisor(configurer)
{
}

CustomViewerWorkbenchWindowAdvisor::~CustomViewerWorkbenchWindowAdvisor()
{
}

void CustomViewerWorkbenchWindowAdvisor::PostWindowCreate()
{
  //for stylesheet customization
  //static_cast<QMainWindow*>(this->GetWindowConfigurer()->GetWindow()->GetShell()->GetControl())->dumpObjectTree();
}

// //! [CustomViewerWorkbenchWindowAdvisorPreWindowOpen]
void CustomViewerWorkbenchWindowAdvisor::PreWindowOpen()
{
  berry::IWorkbenchWindowConfigurer::Pointer configurer = this->GetWindowConfigurer();
  configurer->SetTitle("Spartan Viewer");
  configurer->SetShowMenuBar(false);
  configurer->SetShowCoolBar(false);
  configurer->SetShowPerspectiveBar(false);
  configurer->SetShowStatusLine(false);
}
// //! [CustomViewerWorkbenchWindowAdvisorPreWindowOpen]
// //! [WorkbenchWindowAdvisorCreateWindowContentsHead]
void CustomViewerWorkbenchWindowAdvisor::CreateWindowContents(berry::Shell::Pointer shell)
{
  //the all containing main window
  QMainWindow* mainWindow = static_cast<QMainWindow*>(shell->GetControl());
// //! [WorkbenchWindowAdvisorCreateWindowContentsHead]
  mainWindow->setVisible(true);

  //the widgets
  QWidget* CentralWidget = new QWidget(mainWindow);
  CentralWidget->setObjectName("CentralWidget");
  CentralWidget->setVisible(true);

  QtPerspectiveSwitcherTabBar* PerspectivesTabBar = new QtPerspectiveSwitcherTabBar(this->GetWindowConfigurer()->GetWindow());
  PerspectivesTabBar->setObjectName("PerspectivesTabBar");
  PerspectivesTabBar->addTab("Image Viewer");
  PerspectivesTabBar->addTab("DICOM-Manager");
  PerspectivesTabBar->setVisible(true);
  PerspectivesTabBar->setDrawBase(false);

  QPushButton* StyleUpdateButton = new QPushButton("Update Style", mainWindow);
  StyleUpdateButton->setMaximumWidth(100);
  StyleUpdateButton->setObjectName("StyleUpdateButton");
  QObject::connect(StyleUpdateButton, SIGNAL( clicked() ), this, SLOT( UpdateStyle() ));

  QToolButton* OpenFileButton = new QToolButton(mainWindow);
  OpenFileButton->setMaximumWidth(100);
  OpenFileButton->setObjectName("FileOpenButton");
  OpenFileButton->setText("Open File");
  QObject::connect(OpenFileButton, SIGNAL( clicked() ), this, SLOT( OpenFile() ));

  QWidget* PageComposite = new QWidget(CentralWidget);
  PageComposite->setObjectName("PageComposite");
  PageComposite->setVisible(true);

  //the layouts
  QVBoxLayout* CentralWidgetLayout = new QVBoxLayout(CentralWidget);
  CentralWidgetLayout->contentsMargins();
  CentralWidgetLayout->setContentsMargins(9,9,9,9);
  CentralWidgetLayout->setSpacing(0);
  CentralWidgetLayout->setObjectName("CentralWidgetLayout");

  QHBoxLayout* PerspectivesLayer = new QHBoxLayout(mainWindow);
  PerspectivesLayer->setObjectName("PerspectivesLayer");

  QHBoxLayout* PageCompositeLayout = new QHBoxLayout(PageComposite);
  PageCompositeLayout->setContentsMargins(0,0,0,0);
  PageCompositeLayout->setSpacing(0);
  PageComposite->setLayout(PageCompositeLayout);
// //! [WorkbenchWindowAdvisorCreateWindowContents]
  //all glued together
  mainWindow->setCentralWidget(CentralWidget);
  CentralWidgetLayout->addLayout(PerspectivesLayer);
  CentralWidgetLayout->addWidget(PageComposite);
  CentralWidget->setLayout(CentralWidgetLayout);
  PerspectivesLayer->addWidget(PerspectivesTabBar);
  PerspectivesLayer->addSpacing(300);
  PerspectivesLayer->addWidget(OpenFileButton);

  //for style customization convenience
  /*PerspectivesLayer->addSpacing(10);
  PerspectivesLayer->addWidget(StyleUpdateButton);*/

  //for correct initial layout, see also bug#1654
  CentralWidgetLayout->activate();
  CentralWidgetLayout->update();

  this->GetWindowConfigurer()->CreatePageComposite(static_cast<void*>(PageComposite));
// //! [WorkbenchWindowAdvisorCreateWindowContents]
}
// //! [WorkbenchWindowAdvisorUpdateStyle]
void CustomViewerWorkbenchWindowAdvisor::UpdateStyle()
{
  ctkPluginContext* pluginContext = org_mitk_example_gui_customviewer_Activator::GetPluginContext();
  ctkServiceReference serviceReference = pluginContext->getServiceReference<berry::IQtStyleManager>();

  //granted by org.blueberry.ui.qt
  Q_ASSERT(serviceReference);

  berry::IQtStyleManager* styleManager = pluginContext->getService<berry::IQtStyleManager>(serviceReference);
  Q_ASSERT(styleManager);

  styleManager->SetStyle("/home/me/customstyle.qss");
}
// //! [WorkbenchWindowAdvisorUpdateStyle]
// //! [WorkbenchWindowAdvisorOpenFile]
void CustomViewerWorkbenchWindowAdvisor::OpenFile()
{

  // Ask the user for a list of files to open
  QStringList fileNames = QFileDialog::getOpenFileNames(NULL, "Open",
                                                        /*d->getLastFileOpenPath()*/ QString(),
                                                        mitk::CoreObjectFactory::GetInstance()->GetFileExtensions());

  if (fileNames.empty())
    return;

  mitk::WorkbenchUtil::LoadFiles(fileNames, this->GetWindowConfigurer()->GetWindow(), false);
// //! [WorkbenchWindowAdvisorOpenFile]
// //! [WorkbenchWindowAdvisorOpenFilePerspActive]
  berry::IWorkbenchWindow::Pointer window = this->GetWindowConfigurer()->GetWindow();
  std::string perspectiveId = "org.mitk.example.viewerperspective";
  window->GetWorkbench()->ShowPerspective(perspectiveId, berry::IWorkbenchWindow::Pointer(window));
// //! [WorkbenchWindowAdvisorOpenFilePerspActive]
}
