/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#include "org_mitk_gui_qt_cli_Activator.h"

#include <QtPlugin>

#include "CommandLineModulesView.h"
#include "CommandLineModulesPreferencesPage.h"

namespace mitk {

void org_mitk_gui_qt_cli_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(CommandLineModulesView, context)
  BERRY_REGISTER_EXTENSION_CLASS(CommandLineModulesPreferencesPage, context)
}

void org_mitk_gui_qt_cli_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_cli, mitk::org_mitk_gui_qt_cli_Activator)
