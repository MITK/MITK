/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "CustomViewerWorkbenchAdvisor.h"
#include "CustomViewerWorkbenchWindowAdvisor.h"
#include "berryIQtStyleManager.h"
#include "org_mitk_example_gui_customviewer_Activator.h"

const QString CustomViewerWorkbenchAdvisor::DEFAULT_PERSPECTIVE_ID = "org.mitk.example.viewerperspective";

// //! [WorkbenchAdvisorCreateWindowAdvisor]
berry::WorkbenchWindowAdvisor *CustomViewerWorkbenchAdvisor::CreateWorkbenchWindowAdvisor(
  berry::IWorkbenchWindowConfigurer::Pointer configurer)
{
  return new CustomViewerWorkbenchWindowAdvisor(configurer);
}
// //! [WorkbenchAdvisorCreateWindowAdvisor]

CustomViewerWorkbenchAdvisor::~CustomViewerWorkbenchAdvisor()
{
}
// //! [WorkbenchAdvisorInit]
void CustomViewerWorkbenchAdvisor::Initialize(berry::IWorkbenchConfigurer::Pointer configurer)
{
  berry::QtWorkbenchAdvisor::Initialize(configurer);

  ctkPluginContext *pluginContext = org_mitk_example_gui_customviewer_Activator::GetPluginContext();
  ctkServiceReference serviceReference = pluginContext->getServiceReference<berry::IQtStyleManager>();

  // always granted by org.blueberry.ui.qt
  Q_ASSERT(serviceReference);

  berry::IQtStyleManager *styleManager = pluginContext->getService<berry::IQtStyleManager>(serviceReference);
  Q_ASSERT(styleManager);

  QString styleName = "CustomStyle";
  styleManager->AddStyle(":/customstyle.qss", styleName);
  styleManager->SetStyle(":/customstyle.qss");
  // styleManager->AddStyle("/home/me/customstyle.qss", styleName);
  // styleManager->SetStyle("/home/me/customstyle.qss");
}
// //! [WorkbenchAdvisorInit]
QString CustomViewerWorkbenchAdvisor::GetInitialWindowPerspectiveId()
{
  return DEFAULT_PERSPECTIVE_ID;
}
