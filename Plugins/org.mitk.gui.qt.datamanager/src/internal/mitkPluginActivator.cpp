/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "mitkPluginActivator.h"
#include "../QmitkDataManagerHotkeysPrefPage.h"
#include "../QmitkDataManagerPreferencePage.h"
#include "../QmitkDataManagerView.h"

#include <usModuleInitialization.h>

US_INITIALIZE_MODULE

namespace mitk {

void PluginActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDataManagerView, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDataManagerPreferencePage, context)
  BERRY_REGISTER_EXTENSION_CLASS(QmitkDataManagerHotkeysPrefPage, context)
}

void PluginActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}
