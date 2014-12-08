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
  BERRY_REGISTER_EXTENSION_CLASS(QmitkWelcomePerspective, context)


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

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_mitk_gui_qt_diffusionimagingapp, QmitkDiffusionApplicationPlugin)
#endif
