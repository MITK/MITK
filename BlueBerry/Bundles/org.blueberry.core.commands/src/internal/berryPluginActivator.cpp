/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#include "berryPluginActivator.h"

#include <QtPlugin>

namespace berry {

org_blueberry_core_commands_Activator::org_blueberry_core_commands_Activator()
{
}

void org_blueberry_core_commands_Activator::start(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

void org_blueberry_core_commands_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_blueberry_core_commands, berry::org_blueberry_core_commands_Activator)
#endif
