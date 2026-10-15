/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPluginActivator.h"
#include "QmitkMxNDemoView.h"

#include <usModuleInitialization.h>

US_INITIALIZE_MODULE

namespace mitk
{
  void PluginActivator::start(ctkPluginContext* context)
  {
    // The editor and the preference page are classes of the MxN editor plugin,
    // which registers them itself; only this plugin's own view is registered.
    BERRY_REGISTER_EXTENSION_CLASS(QmitkMxNDemoView, context)
  }

  void PluginActivator::stop(ctkPluginContext*)
  {
  }
}
