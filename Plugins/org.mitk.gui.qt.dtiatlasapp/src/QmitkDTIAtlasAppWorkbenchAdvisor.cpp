/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (cGerman Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkDTIAtlasAppWorkbenchAdvisor.h"

#include "internal/QmitkDTIAtlasAppApplicationPlugin.h"

#include <berryPlatform.h>
#include <berryIPreferencesService.h>
#include <berryIPreferences.h>
#include <berryWorkbenchPreferenceConstants.h>

#include <QmitkExtWorkbenchWindowAdvisor.h>

#include <QApplication>
#include <QPoint>

const QString QmitkDTIAtlasAppWorkbenchAdvisor::WELCOME_PERSPECTIVE_ID = "org.mitk.dtiatlasapp.perspectives.welcome";


void
QmitkDTIAtlasAppWorkbenchAdvisor::Initialize(berry::IWorkbenchConfigurer::Pointer configurer)
{
  berry::QtWorkbenchAdvisor::Initialize(configurer);

  configurer->SetSaveAndRestore(true);

  // TODO This should go into the products plugin_customization.ini file (when
  // the product and branding support is finished, see bug 2146).
  // This will not work anymore, if bug 2822 is fixed.
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
  prefService->GetSystemPreferences()->Put(berry::WorkbenchPreferenceConstants::PREFERRED_SASH_LAYOUT, berry::WorkbenchPreferenceConstants::RIGHT);
}

berry::WorkbenchWindowAdvisor*
QmitkDTIAtlasAppWorkbenchAdvisor::CreateWorkbenchWindowAdvisor(
        berry::IWorkbenchWindowConfigurer::Pointer configurer)
{
  QList<QString> perspExcludeList;
  perspExcludeList.push_back( "org.mitk.dtiatlasapp.perspectives.welcome");
  perspExcludeList.push_back( "org.mitk.perspectives.diffusionimaginginternal");
  perspExcludeList.push_back( "org.mitk.perspectives.publicdiffusionimaging");
  perspExcludeList.push_back( "org.mitk.extapp.defaultperspective");
  perspExcludeList.push_back( "org.mitk.coreapp.defaultperspective");

  QList<QString> viewExcludeList;
  viewExcludeList.push_back( "org.mitk.views.partialvolumeanalysis");
  viewExcludeList.push_back( "org.mitk.views.globalfibertracking");
  viewExcludeList.push_back( "org.mitk.views.tractbasedspatialstatistics");
  viewExcludeList.push_back( "org.mitk.views.fibertracking");
  viewExcludeList.push_back( "org.mitk.views.ivim");
  viewExcludeList.push_back( "org.mitk.views.qballreconstruction");
  viewExcludeList.push_back( "org.mitk.views.diffusiondicomimport");
  viewExcludeList.push_back( "org.mitk.views.diffusionpreprocessing");
  viewExcludeList.push_back( "org.mitk.views.diffusionquantification");
  viewExcludeList.push_back( "org.mitk.views.tensorreconstruction");
  viewExcludeList.push_back( "org.mitk.views.perspectiveswitcher");
  viewExcludeList.push_back( "org.mitk.views.basicimageprocessing");
  viewExcludeList.push_back( "org.mitk.views.fiberbundleoperations");
  viewExcludeList.push_back( "org.mitk.views.measurement");
  viewExcludeList.push_back( "org.mitk.views.moviemaker");
  viewExcludeList.push_back( "org.mitk.views.odfdetails");
  viewExcludeList.push_back( "org.mitk.views.properties");
  viewExcludeList.push_back( "org.mitk.views.screenshotmaker");
  viewExcludeList.push_back( "org.mitk.views.segmentation");
  viewExcludeList.push_back( "org.mitk.views.imagestatistics");
//  viewExcludeList.push_back( "org.mitk.views.controlvisualizationpropertiesview");
  viewExcludeList.push_back( "org.mitk.views.volumevisualization");
  viewExcludeList.push_back( "org.mitk.views.simplemeasurement");

  configurer->SetShowPerspectiveBar(false);
  configurer->SetInitialSize(QPoint(1000,770));

  QmitkExtWorkbenchWindowAdvisor* advisor = new QmitkExtWorkbenchWindowAdvisor(this, configurer);
  advisor->SetPerspectiveExcludeList(perspExcludeList);
  advisor->SetViewExcludeList(viewExcludeList);
  advisor->ShowViewToolbar(false);
  advisor->ShowVersionInfo(false);
  advisor->ShowMitkVersionInfo(false);
  advisor->SetProductName("based on MITK Diffusion Imaging App");
  advisor->SetWindowIcon(":/org.mitk.gui.qt.dtiatlasapp/app-icon.png");
  return advisor;
}

QString QmitkDTIAtlasAppWorkbenchAdvisor::GetInitialWindowPerspectiveId()
{
  return WELCOME_PERSPECTIVE_ID;
}
