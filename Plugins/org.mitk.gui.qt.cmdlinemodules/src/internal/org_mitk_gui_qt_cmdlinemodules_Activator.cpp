/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "org_mitk_gui_qt_cmdlinemodules_Activator.h"
#include "CommandLineModulesView.h"
#include "CommandLineModulesPreferencesPage.h"

#include <usModuleInitialization.h>

US_INITIALIZE_MODULE

namespace mitk {

void org_mitk_gui_qt_cmdlinemodules_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(CommandLineModulesView, context)
  BERRY_REGISTER_EXTENSION_CLASS(CommandLineModulesPreferencesPage, context)
}

void org_mitk_gui_qt_cmdlinemodules_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}
