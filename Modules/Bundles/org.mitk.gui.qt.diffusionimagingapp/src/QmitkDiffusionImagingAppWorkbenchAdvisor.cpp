/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date: 2009-10-23 02:59:36 +0200 (Fri, 23 Oct 2009) $
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "QmitkDiffusionImagingAppWorkbenchAdvisor.h"

#include "internal/mitkPluginActivator.h"

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

  // TODO This should go into the products plugin_customization.ini file (when
  // the product and branding support is finished, see bug 2146).
  // This will not work anymore, if bug 2822 is fixed.
  berry::IPreferencesService::Pointer prefService = berry::Platform::GetServiceRegistry().GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);
  prefService->GetSystemPreferences()->Put(berry::WorkbenchPreferenceConstants::PREFERRED_SASH_LAYOUT, berry::WorkbenchPreferenceConstants::RIGHT);

  QString collectionFile = mitkPluginActivator::GetDefault()->GetQtHelpCollectionFile();
  if (!collectionFile.isEmpty())
  {
//    berry::QtAssistantUtil::SetHelpCollectionFile(collectionFile);
//    berry::QtAssistantUtil::SetDefaultHelpUrl("qthelp://org.mitk.gui.qt.diffusionimagingapp/bundle/index.html");

    typedef std::vector<berry::IBundle::Pointer> BundleContainer;
    BundleContainer bundles = berry::Platform::GetBundles();
    berry::QtAssistantUtil::RegisterQCHFiles(bundles);
  }

}

berry::WorkbenchWindowAdvisor*
QmitkDiffusionImagingAppWorkbenchAdvisor::CreateWorkbenchWindowAdvisor(
        berry::IWorkbenchWindowConfigurer::Pointer configurer)
{
  std::vector<std::string> perspExcludeList;
  perspExcludeList.push_back( std::string("org.blueberry.uitest.util.EmptyPerspective") );
  perspExcludeList.push_back( std::string("org.blueberry.uitest.util.EmptyPerspective2") );
  perspExcludeList.push_back( std::string("org.mitk.coreapp.defaultperspective") );
  perspExcludeList.push_back( std::string("org.mitk.extapp.defaultperspective") );
  perspExcludeList.push_back( std::string("org.mitk.perspectives.publicdiffusionimaging") );
  perspExcludeList.push_back( std::string("org.mitk.perspectives.diffusionimaginginternal") );

  std::vector<std::string> viewExcludeList;
//  viewExcludeList.push_back( std::string("org.mitk.views.partialvolumeanalysis") );
//  viewExcludeList.push_back( std::string("org.mitk.views.globalfibertracking") );
//  viewExcludeList.push_back( std::string("org.mitk.views.tractbasedspatialstatistics") );
//  viewExcludeList.push_back( std::string("org.mitk.views.fibertracking") );
//  viewExcludeList.push_back( std::string("org.mitk.views.ivim") );
//  viewExcludeList.push_back( std::string("org.mitk.views.qballreconstruction") );
//  viewExcludeList.push_back( std::string("org.mitk.views.diffusiondicomimport") );
//  viewExcludeList.push_back( std::string("org.mitk.views.diffusionpreprocessing") );
//  viewExcludeList.push_back( std::string("org.mitk.views.diffusionquantification") );
//  viewExcludeList.push_back( std::string("org.mitk.views.tensorreconstruction") );
//  viewExcludeList.push_back( std::string("org.mitk.views.perspectiveswitcher") );
//  viewExcludeList.push_back( std::string("org.mitk.views.basicimageprocessing") );
//  viewExcludeList.push_back( std::string("org.mitk.views.fiberbundleoperations") );
//  viewExcludeList.push_back( std::string("org.mitk.views.measurement") );
//  viewExcludeList.push_back( std::string("org.mitk.views.moviemaker") );
//  viewExcludeList.push_back( std::string("org.mitk.views.odfdetails") );
//  viewExcludeList.push_back( std::string("org.mitk.views.propertylistview") );
//  viewExcludeList.push_back( std::string("org.mitk.views.screenshotmaker") );
//  viewExcludeList.push_back( std::string("org.mitk.views.segmentation") );
//  viewExcludeList.push_back( std::string("org.mitk.views.imagestatistics") );
//  viewExcludeList.push_back( std::string("org.mitk.views.controlvisualizationpropertiesview") );
//  viewExcludeList.push_back( std::string("org.mitk.views.volumevisualization") );
//  viewExcludeList.push_back( std::string("org.mitk.views.simplemeasurement") );

  configurer->SetInitialSize(berry::Point(1000,770));

  QmitkExtWorkbenchWindowAdvisor* advisor = new QmitkExtWorkbenchWindowAdvisor(this, configurer);
  advisor->ShowViewMenuItem(false);
  advisor->ShowNewWindowMenuItem(false);
  advisor->ShowClosePerspectiveMenuItem(false);
  advisor->SetPerspectiveExcludeList(perspExcludeList);
  advisor->SetViewExcludeList(viewExcludeList);
  advisor->ShowViewToolbar(false);
  advisor->ShowPerspectiveToolbar(true);
  advisor->ShowVersionInfo(false);
  advisor->ShowMitkVersionInfo(false);
  advisor->SetProductName("MITK Diffusion");
  advisor->SetWindowIcon(":/org.mitk.gui.qt.diffusionimagingapp/app-icon.png");
  return advisor;
}

std::string QmitkDiffusionImagingAppWorkbenchAdvisor::GetInitialWindowPerspectiveId()
{
  return WELCOME_PERSPECTIVE_ID;
}
