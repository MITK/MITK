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

#include "QmitkDTIAtlasAppWorkbenchAdvisor.h"

#include "internal/QmitkDTIAtlasAppApplicationPlugin.h"

#include <berryPlatform.h>
#include <berryIPreferencesService.h>
#include <berryWorkbenchPreferenceConstants.h>
#include <berryQtAssistantUtil.h>
#include <QmitkExtWorkbenchWindowAdvisor.h>

#include <QApplication>

const std::string QmitkDTIAtlasAppWorkbenchAdvisor::WELCOME_PERSPECTIVE_ID = "org.mitk.dtiatlasapp.perspectives.welcome";


void
QmitkDTIAtlasAppWorkbenchAdvisor::Initialize(berry::IWorkbenchConfigurer::Pointer configurer)
{
  berry::QtWorkbenchAdvisor::Initialize(configurer);

  configurer->SetSaveAndRestore(true);

  // TODO This should go into the products plugin_customization.ini file (when
  // the product and branding support is finished, see bug 2146).
  // This will not work anymore, if bug 2822 is fixed.
  berry::IPreferencesService::Pointer prefService = berry::Platform::GetServiceRegistry().GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);
  prefService->GetSystemPreferences()->Put(berry::WorkbenchPreferenceConstants::PREFERRED_SASH_LAYOUT, berry::WorkbenchPreferenceConstants::RIGHT);

  QString collectionFile = QmitkDTIAtlasAppApplicationPlugin::GetDefault()->GetQtHelpCollectionFile();
  if (!collectionFile.isEmpty())
  {
//    berry::QtAssistantUtil::SetHelpCollectionFile(collectionFile);
//    berry::QtAssistantUtil::SetDefaultHelpUrl("qthelp://org.mitk.gui.qt.dtiatlasapp/bundle/index.html");

    typedef std::vector<berry::IBundle::Pointer> BundleContainer;
    BundleContainer bundles = berry::Platform::GetBundles();
    berry::QtAssistantUtil::RegisterQCHFiles(bundles);
  }

}

berry::WorkbenchWindowAdvisor*
QmitkDTIAtlasAppWorkbenchAdvisor::CreateWorkbenchWindowAdvisor(
        berry::IWorkbenchWindowConfigurer::Pointer configurer)
{
  std::vector<std::string> perspExcludeList;
  perspExcludeList.push_back( std::string("org.mitk.dtiatlasapp.perspectives.welcome") );
  perspExcludeList.push_back( std::string("org.mitk.perspectives.diffusionimaginginternal") );
  perspExcludeList.push_back( std::string("org.mitk.perspectives.publicdiffusionimaging") );
  perspExcludeList.push_back( std::string("org.mitk.extapp.defaultperspective") );
  perspExcludeList.push_back( std::string("org.mitk.coreapp.defaultperspective") );

  std::vector<std::string> viewExcludeList;
  viewExcludeList.push_back( std::string("org.mitk.views.partialvolumeanalysis") );
  viewExcludeList.push_back( std::string("org.mitk.views.globalfibertracking") );
  viewExcludeList.push_back( std::string("org.mitk.views.tractbasedspatialstatistics") );
  viewExcludeList.push_back( std::string("org.mitk.views.fibertracking") );
  viewExcludeList.push_back( std::string("org.mitk.views.ivim") );
  viewExcludeList.push_back( std::string("org.mitk.views.qballreconstruction") );
  viewExcludeList.push_back( std::string("org.mitk.views.diffusiondicomimport") );
  viewExcludeList.push_back( std::string("org.mitk.views.diffusionpreprocessing") );
  viewExcludeList.push_back( std::string("org.mitk.views.diffusionquantification") );
  viewExcludeList.push_back( std::string("org.mitk.views.tensorreconstruction") );
  viewExcludeList.push_back( std::string("org.mitk.views.perspectiveswitcher") );
  viewExcludeList.push_back( std::string("org.mitk.views.basicimageprocessing") );
  viewExcludeList.push_back( std::string("org.mitk.views.fiberbundleoperations") );
  viewExcludeList.push_back( std::string("org.mitk.views.measurement") );
  viewExcludeList.push_back( std::string("org.mitk.views.moviemaker") );
  viewExcludeList.push_back( std::string("org.mitk.views.odfdetails") );
  viewExcludeList.push_back( std::string("org.mitk.views.properties") );
  viewExcludeList.push_back( std::string("org.mitk.views.screenshotmaker") );
  viewExcludeList.push_back( std::string("org.mitk.views.segmentation") );
  viewExcludeList.push_back( std::string("org.mitk.views.imagestatistics") );
//  viewExcludeList.push_back( std::string("org.mitk.views.controlvisualizationpropertiesview") );
  viewExcludeList.push_back( std::string("org.mitk.views.volumevisualization") );
  viewExcludeList.push_back( std::string("org.mitk.views.simplemeasurement") );

  configurer->SetShowPerspectiveBar(false);
  configurer->SetInitialSize(berry::Point(1000,770));

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

std::string QmitkDTIAtlasAppWorkbenchAdvisor::GetInitialWindowPerspectiveId()
{
  return WELCOME_PERSPECTIVE_ID;
}
