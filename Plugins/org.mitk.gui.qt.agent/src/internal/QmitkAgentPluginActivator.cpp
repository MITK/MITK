/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkAgentPluginActivator.h"
#include "QmitkAgentPreferencePage.h"
#include "QmitkAgentView.h"

#include <usModuleInitialization.h>

US_INITIALIZE_MODULE

ctkPluginContext* QmitkAgentPluginActivator::s_Context = nullptr;
QmitkAgentPluginActivator* QmitkAgentPluginActivator::s_Instance = nullptr;

void QmitkAgentPluginActivator::start(ctkPluginContext* context)
{
  s_Context = context;
  s_Instance = this;
  BERRY_REGISTER_EXTENSION_CLASS(QmitkAgentPreferencePage, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkAgentView, context)
}

void QmitkAgentPluginActivator::stop(ctkPluginContext*)
{
  s_Context = nullptr;
  s_Instance = nullptr;
}

ctkPluginContext* QmitkAgentPluginActivator::GetContext()
{
  return s_Context;
}

QmitkAgentPluginActivator* QmitkAgentPluginActivator::GetInstance()
{
  return s_Instance;
}
