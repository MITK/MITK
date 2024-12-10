/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPluginActivator.h"
#include "QmitkDicomBrowser.h"

#include <usModuleInitialization.h>

US_INITIALIZE_MODULE

ctkPluginContext* mitk::PluginActivator::s_Context = nullptr;

ctkPluginContext* mitk::PluginActivator::GetContext()
{
  return s_Context;
}

void mitk::PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDicomBrowser, context)
  s_Context = context;
}

void mitk::PluginActivator::stop(ctkPluginContext*)
{
  s_Context = nullptr;
}
