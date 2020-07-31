/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkExtApplicationPlugin.h"
#include "perspectives/QmitkExtDefaultPerspective.h"
#include "perspectives/QmitkEditorPerspective.h"
#include "perspectives/QmitkVisualizationPerspective.h"
#include "QmitkExtApplication.h"

#include <mitkVersion.h>
#include <mitkLogMacros.h>

#include <service/cm/ctkConfigurationAdmin.h>
#include <service/cm/ctkConfiguration.h>

#include <QFileInfo>
#include <QDateTime>

QmitkExtApplicationPlugin* QmitkExtApplicationPlugin::inst = nullptr;

QmitkExtApplicationPlugin::QmitkExtApplicationPlugin()
{
  inst = this;
}

QmitkExtApplicationPlugin::~QmitkExtApplicationPlugin()
{
}

QmitkExtApplicationPlugin* QmitkExtApplicationPlugin::GetDefault()
{
  return inst;
}

void QmitkExtApplicationPlugin::start(ctkPluginContext* context)
{
  berry::AbstractUICTKPlugin::start(context);

  this->context = context;

  BERRY_REGISTER_EXTENSION_CLASS(QmitkExtDefaultPerspective, context);
  BERRY_REGISTER_EXTENSION_CLASS(QmitkEditorPerspective, context);
  BERRY_REGISTER_EXTENSION_CLASS(QmitkExtApplication, context);
  BERRY_REGISTER_EXTENSION_CLASS(QmitkVisualizationPerspective, context);


  ctkServiceReference cmRef = context->getServiceReference<ctkConfigurationAdmin>();
  ctkConfigurationAdmin* configAdmin = nullptr;
  if (cmRef)
  {
    configAdmin = context->getService<ctkConfigurationAdmin>(cmRef);
  }

  // Use the CTK Configuration Admin service to configure the BlueBerry help system
  if (configAdmin)
  {
    ctkConfigurationPtr conf = configAdmin->getConfiguration("org.blueberry.services.help", QString());
    ctkDictionary helpProps;
    helpProps.insert("homePage", "qthelp://org.mitk.gui.qt.extapplication/bundle/index.html");
    conf->update(helpProps);
    context->ungetService(cmRef);
  }
  else
  {
    MITK_WARN << "Configuration Admin service unavailable, cannot set home page url.";
  }
}

ctkPluginContext* QmitkExtApplicationPlugin::GetPluginContext() const
{
  return context;
}
