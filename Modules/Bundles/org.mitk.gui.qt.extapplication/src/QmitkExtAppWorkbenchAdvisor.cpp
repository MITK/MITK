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

#include "QmitkExtAppWorkbenchAdvisor.h"
#include "internal/QmitkExtApplicationPlugin.h"

#include <cherryQtAssistantUtil.h>
#include <QmitkExtWorkbenchWindowAdvisor.h>

const std::string QmitkExtAppWorkbenchAdvisor::DEFAULT_PERSPECTIVE_ID =
    "org.mitk.extapp.defaultperspective";

void
QmitkExtAppWorkbenchAdvisor::Initialize(cherry::IWorkbenchConfigurer::Pointer configurer)
{
  cherry::QtWorkbenchAdvisor::Initialize(configurer);

  configurer->SetSaveAndRestore(true);

  QString collectionFile = QmitkExtApplicationPlugin::GetDefault()->GetQtHelpCollectionFile();
  if (!collectionFile.isEmpty())
  {
    cherry::IBundleContext::Pointer context = QmitkExtApplicationPlugin::GetDefault()->GetBundleContext();
    typedef std::vector<cherry::IBundle::Pointer> BundleContainer;
    BundleContainer bundles;
    context->ListBundles(bundles);
    cherry::QtAssistantUtil::RegisterQCHFiles(collectionFile, bundles);
  }

  cherry::QtAssistantUtil::SetHelpColletionFile(collectionFile);
  cherry::QtAssistantUtil::SetDefaultHelpUrl("qthelp://org.mitk.gui.qt.extapplication/bundle/index.html");
}

cherry::WorkbenchWindowAdvisor*
QmitkExtAppWorkbenchAdvisor::CreateWorkbenchWindowAdvisor(
        cherry::IWorkbenchWindowConfigurer::Pointer configurer)
{
  return new QmitkExtWorkbenchWindowAdvisor(this, configurer);
}

std::string QmitkExtAppWorkbenchAdvisor::GetInitialWindowPerspectiveId()
{
  return DEFAULT_PERSPECTIVE_ID;
}
