/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "QmitkFlowApplicationWorkbenchAdvisor.h"
#include "internal/QmitkFlowApplicationPlugin.h"

#include "QmitkFlowApplicationWorkbenchWindowAdvisor.h"

const QString QmitkFlowApplicationWorkbenchAdvisor::DEFAULT_PERSPECTIVE_ID =
    "org.mitk.qt.flowapplication.defaultperspective";

void
QmitkFlowApplicationWorkbenchAdvisor::Initialize(berry::IWorkbenchConfigurer::Pointer configurer)
{
  berry::QtWorkbenchAdvisor::Initialize(configurer);

  configurer->SetSaveAndRestore(true);
}

berry::WorkbenchWindowAdvisor*
QmitkFlowApplicationWorkbenchAdvisor::CreateWorkbenchWindowAdvisor(
        berry::IWorkbenchWindowConfigurer::Pointer configurer)
{
  QmitkFlowApplicationWorkbenchWindowAdvisor* advisor = new
    QmitkFlowApplicationWorkbenchWindowAdvisor(this, configurer);

  // Exclude the help perspective from org.blueberry.ui.qt.help from
  // the normal perspective list.
  // The perspective gets a dedicated menu entry in the help menu
  QList<QString> excludePerspectives;
  excludePerspectives.push_back("org.blueberry.perspectives.help");
  advisor->SetPerspectiveExcludeList(excludePerspectives);

  // Exclude some views from the normal view list
  QList<QString> excludeViews;
  excludeViews.push_back("org.mitk.views.modules");
  excludeViews.push_back( "org.blueberry.ui.internal.introview" );
  advisor->SetViewExcludeList(excludeViews);

  advisor->SetWindowIcon(":/org.mitk.gui.qt.flowapplication/icon.png");
  return advisor;
}

QString QmitkFlowApplicationWorkbenchAdvisor::GetInitialWindowPerspectiveId()
{
  return DEFAULT_PERSPECTIVE_ID;
}
