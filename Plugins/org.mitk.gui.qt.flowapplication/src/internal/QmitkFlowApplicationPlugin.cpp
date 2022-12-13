/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkFlowApplicationPlugin.h"
#include "QmitkFlowApplication.h"

#include <berryMacros.h>

#include <QtWidgetsExtRegisterClasses.h>

#include <service/cm/ctkConfigurationAdmin.h>

#include <usModuleInitialization.h>

US_INITIALIZE_MODULE

QmitkFlowApplicationPlugin* QmitkFlowApplicationPlugin::inst = nullptr;

QmitkFlowApplicationPlugin::QmitkFlowApplicationPlugin()
  : _context(nullptr)
{
  inst = this;
}

QmitkFlowApplicationPlugin::~QmitkFlowApplicationPlugin()
{
}

QmitkFlowApplicationPlugin* QmitkFlowApplicationPlugin::GetDefault()
{
  return inst;
}

void QmitkFlowApplicationPlugin::start(ctkPluginContext* context)
{
  berry::AbstractUICTKPlugin::start(context);

  this->_context = context;

  QtWidgetsExtRegisterClasses();

  BERRY_REGISTER_EXTENSION_CLASS(QmitkFlowApplication, context);

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
    helpProps.insert("homePage", "qthelp://org.mitk.gui.qt.flowapplication/bundle/index.html");
    conf->update(helpProps);
    context->ungetService(cmRef);
  }
  else
  {
    MITK_WARN << "Configuration Admin service unavailable, cannot set home page url.";
  }
}

void QmitkFlowApplicationPlugin::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)

    this->_context = nullptr;
}

ctkPluginContext* QmitkFlowApplicationPlugin::GetPluginContext() const
{
  return _context;
}
