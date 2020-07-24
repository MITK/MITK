/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPluginActivator.h"
#include "QmitkDicomBrowser.h"
#include "QmitkDicomPreferencePage.h"
#include <usModuleInitialization.h>

US_INITIALIZE_MODULE

namespace mitk {
ctkPluginContext* PluginActivator::pluginContext = nullptr;

void PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDicomBrowser, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDicomPreferencePage, context)
  pluginContext = context;
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
  pluginContext = nullptr;
}
ctkPluginContext* PluginActivator::getContext()
{
    return pluginContext;
}
}
