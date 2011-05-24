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

#include <berryQtAssistantUtil.h>
#include <QmitkExtWorkbenchWindowAdvisor.h>

const std::string QmitkExtAppWorkbenchAdvisor::DEFAULT_PERSPECTIVE_ID =
    "org.mitk.extapp.defaultperspective";

void
QmitkExtAppWorkbenchAdvisor::Initialize(berry::IWorkbenchConfigurer::Pointer configurer)
{
  berry::QtWorkbenchAdvisor::Initialize(configurer);

  configurer->SetSaveAndRestore(true);

  QString collectionFile = QmitkExtApplicationPlugin::GetDefault()->GetQtHelpCollectionFile();
  if (!collectionFile.isEmpty())
  {
    ctkPluginContext* context = QmitkExtApplicationPlugin::GetDefault()->GetPluginContext();
    typedef std::vector<berry::IBundle::Pointer> BundleContainer;
    BundleContainer bundles = berry::Platform::GetBundles();
    QList<QSharedPointer<ctkPlugin> > plugins = context->getPlugins();
    berry::QtAssistantUtil::RegisterQCHFiles(collectionFile, bundles, plugins);
  }

  berry::QtAssistantUtil::SetHelpColletionFile(collectionFile);
  berry::QtAssistantUtil::SetDefaultHelpUrl("qthelp://org.mitk.gui.qt.extapplication/bundle/index.html");
}

berry::WorkbenchWindowAdvisor*
QmitkExtAppWorkbenchAdvisor::CreateWorkbenchWindowAdvisor(
        berry::IWorkbenchWindowConfigurer::Pointer configurer)
{
  QmitkExtWorkbenchWindowAdvisor* advisor = new
    QmitkExtWorkbenchWindowAdvisor(this, configurer);
  advisor->SetWindowIcon(":/QmitkExtApplication/icon_research.xpm");
  return advisor;
  //return new QmitkExtWorkbenchWindowAdvisor(this, configurer);
}

std::string QmitkExtAppWorkbenchAdvisor::GetInitialWindowPerspectiveId()
{
  return DEFAULT_PERSPECTIVE_ID;
}
