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

#include "QmitkDiffusionApplicationPlugin.h"
#include "src/QmitkDiffusionImagingAppApplication.h"

#include "src/internal/Perspectives/QmitkWelcomePerspective.h"
#include "src/internal/QmitkDiffusionImagingAppIntroPart.h"
#include "src/internal/Perspectives/QmitkDiffusionImagingAppPerspective.h"

#include "src/internal/Perspectives/QmitkDIAppConnectomicsPerspective.h"
#include "src/internal/Perspectives/QmitkDIAppDicomImportPerspective.h"
#include "src/internal/Perspectives/QmitkDIAppFiberTractographyPerspective.h"
#include "src/internal/Perspectives/QmitkDIAppIVIMPerspective.h"
#include "src/internal/Perspectives/QmitkDIAppImageProcessingPerspective.h"
#include "src/internal/Perspectives/QmitkDIAppPreprocessingReconstructionPerspective.h"
#include "src/internal/Perspectives/QmitkDIAppQuantificationPerspective.h"
#include "src/internal/Perspectives/QmitkDIAppTBSSPerspective.h"
#include "src/internal/Perspectives/QmitkDIAppUtilityPerspective.h"
#include "src/internal/Perspectives/QmitkDIAppSyntheticDataGenerationPerspective.h"
#include "src/internal/Perspectives/QmitkDIAppRegistrationPerspective.h"
#include "src/internal/Perspectives/QmitkDIAppVisualizationPerspective.h"

#include <mitkVersion.h>
#include <mitkLogMacros.h>

#include <service/cm/ctkConfigurationAdmin.h>
#include <service/cm/ctkConfiguration.h>

#include <QFileInfo>
#include <QDateTime>
#include <QtPlugin>

QmitkDiffusionApplicationPlugin* QmitkDiffusionApplicationPlugin::inst = 0;

QmitkDiffusionApplicationPlugin::QmitkDiffusionApplicationPlugin()
{
  inst = this;
}

QmitkDiffusionApplicationPlugin::~QmitkDiffusionApplicationPlugin()
{
}

QmitkDiffusionApplicationPlugin* QmitkDiffusionApplicationPlugin::GetDefault()
{
  return inst;
}

void QmitkDiffusionApplicationPlugin::start(ctkPluginContext* context)
{
  berry::AbstractUICTKPlugin::start(context);

  this->context = context;

  BERRY_REGISTER_EXTENSION_CLASS(QmitkDiffusionImagingAppApplication, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDiffusionImagingAppIntroPart, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDiffusionImagingAppPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkWelcomePerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDIAppConnectomicsPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDIAppDicomImportPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDIAppImageProcessingPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDIAppFiberTractographyPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDIAppIVIMPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDIAppPreprocessingReconstructionPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDIAppQuantificationPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDIAppTBSSPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDIAppUtilityPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDIAppSyntheticDataGenerationPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDIAppRegistrationPerspective, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDIAppVisualizationPerspective, context)

  ctkServiceReference cmRef = context->getServiceReference<ctkConfigurationAdmin>();
  ctkConfigurationAdmin* configAdmin = 0;
  if (cmRef)
  {
    configAdmin = context->getService<ctkConfigurationAdmin>(cmRef);
  }

  // Use the CTK Configuration Admin service to configure the BlueBerry help system

  if (configAdmin)
  {
    ctkConfigurationPtr conf = configAdmin->getConfiguration("org.blueberry.services.help", QString());
    ctkDictionary helpProps;
    helpProps.insert("homePage", "qthelp://org.mitk.gui.qt.diffusionimagingapp/bundle/index.html");
    conf->update(helpProps);
    context->ungetService(cmRef);
  }
  else
  {
    MITK_WARN << "Configuration Admin service unavailable, cannot set home page url.";
  }
}

ctkPluginContext* QmitkDiffusionApplicationPlugin::GetPluginContext() const
{
  return context;
}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_diffusionimagingapp, QmitkDiffusionApplicationPlugin)
