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

#include "QmitkDiffusionImagingAppWorkbenchAdvisor.h"

#include "internal/QmitkDiffusionApplicationPlugin.h"

#include <berryPlatform.h>
#include <berryIPreferencesService.h>
#include <berryWorkbenchPreferenceConstants.h>
#include <QmitkExtWorkbenchWindowAdvisor.h>

#include <QApplication>
#include <QPoint>
#include <QRect>
#include <QDesktopWidget>

const QString QmitkDiffusionImagingAppWorkbenchAdvisor::WELCOME_PERSPECTIVE_ID = "org.mitk.diffusionimagingapp.perspectives.welcome";


void
QmitkDiffusionImagingAppWorkbenchAdvisor::Initialize(berry::IWorkbenchConfigurer::Pointer configurer)
{
  berry::QtWorkbenchAdvisor::Initialize(configurer);

  configurer->SetSaveAndRestore(true);

}

berry::WorkbenchWindowAdvisor*
QmitkDiffusionImagingAppWorkbenchAdvisor::CreateWorkbenchWindowAdvisor(
    berry::IWorkbenchWindowConfigurer::Pointer configurer)
{
  QList<QString> perspExcludeList;
  perspExcludeList.push_back( "org.blueberry.uitest.util.EmptyPerspective" );
  perspExcludeList.push_back( "org.blueberry.uitest.util.EmptyPerspective2" );
  perspExcludeList.push_back("org.blueberry.perspectives.help");

  QList<QString> viewExcludeList;
  viewExcludeList.push_back( "org.mitk.views.controlvisualizationpropertiesview" );

  QRect rec = QApplication::desktop()->screenGeometry();
  configurer->SetInitialSize(QPoint(rec.width(),rec.height()));

  QmitkExtWorkbenchWindowAdvisor* advisor = new QmitkExtWorkbenchWindowAdvisor(this, configurer);
  advisor->ShowViewMenuItem(true);
  advisor->ShowNewWindowMenuItem(true);
  advisor->ShowClosePerspectiveMenuItem(true);
  advisor->SetPerspectiveExcludeList(perspExcludeList);
  advisor->SetViewExcludeList(viewExcludeList);
  advisor->ShowViewToolbar(false);
  advisor->ShowPerspectiveToolbar(true);
  advisor->ShowVersionInfo(true);
  advisor->ShowMitkVersionInfo(true);
  advisor->ShowMemoryIndicator(false);
  advisor->SetProductName("MITK Diffusion");
  advisor->SetWindowIcon(":/org.mitk.gui.qt.diffusionimagingapp/app-icon.png");
  return advisor;
}

QString QmitkDiffusionImagingAppWorkbenchAdvisor::GetInitialWindowPerspectiveId()
{
  return WELCOME_PERSPECTIVE_ID;
}
