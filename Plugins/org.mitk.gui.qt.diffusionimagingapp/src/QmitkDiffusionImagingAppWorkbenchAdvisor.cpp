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
#include <berryQtAssistantUtil.h>
#include <QmitkExtWorkbenchWindowAdvisor.h>

#include <QApplication>

const std::string QmitkDiffusionImagingAppWorkbenchAdvisor::WELCOME_PERSPECTIVE_ID = "org.mitk.diffusionimagingapp.perspectives.welcome";


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
  std::vector<std::string> perspExcludeList;
  perspExcludeList.push_back( std::string("org.blueberry.uitest.util.EmptyPerspective") );
  perspExcludeList.push_back( std::string("org.blueberry.uitest.util.EmptyPerspective2") );
//  perspExcludeList.push_back( std::string("org.mitk.coreapp.defaultperspective") );
  //perspExcludeList.push_back( std::string("org.mitk.extapp.defaultperspective") );
  perspExcludeList.push_back( std::string("org.mitk.perspectives.publicdiffusionimaging") );
  perspExcludeList.push_back( std::string("org.mitk.perspectives.diffusionimaginginternal") );
  // Exclude the help perspective from org.blueberry.ui.qt.help from
  // the normal perspective list.
  // The perspective gets a dedicated menu entry in the help menu
  perspExcludeList.push_back("org.blueberry.perspectives.help");

  std::vector<std::string> viewExcludeList;
  viewExcludeList.push_back( std::string("org.mitk.views.controlvisualizationpropertiesview") );
  viewExcludeList.push_back( std::string("org.mitk.views.imagenavigator") );
//  viewExcludeList.push_back( std::string("org.mitk.views.datamanager") );
  viewExcludeList.push_back( std::string("org.mitk.views.modules") );
  viewExcludeList.push_back( std::string("org.blueberry.ui.internal.introview") );

  configurer->SetInitialSize(berry::Point(1000,770));

  QmitkExtWorkbenchWindowAdvisor* advisor = new QmitkExtWorkbenchWindowAdvisor(this, configurer);
  advisor->ShowViewMenuItem(true);
  advisor->ShowNewWindowMenuItem(true);
  advisor->ShowClosePerspectiveMenuItem(true);
  advisor->SetPerspectiveExcludeList(perspExcludeList);
  advisor->SetViewExcludeList(viewExcludeList);
  advisor->ShowViewToolbar(false);
  advisor->ShowPerspectiveToolbar(false);
  advisor->ShowVersionInfo(false);
  advisor->ShowMitkVersionInfo(false);
  advisor->ShowMemoryIndicator(false);
  advisor->SetProductName("MITK Diffusion");
  advisor->SetWindowIcon(":/org.mitk.gui.qt.diffusionimagingapp/app-icon.png");
  return advisor;
}

std::string QmitkDiffusionImagingAppWorkbenchAdvisor::GetInitialWindowPerspectiveId()
{
  return WELCOME_PERSPECTIVE_ID;
}
