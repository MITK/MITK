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

org_blueberry_core_runtime_Activator::org_blueberry_core_runtime_Activator()
{
}

void org_blueberry_core_runtime_Activator::start(ctkPluginContext* context)
{
  m_PreferencesService = new PreferencesService(context->getDataFile("").absolutePath().toStdString());
  m_PrefServiceReg = context->registerService<IPreferencesService>(m_PreferencesService.GetPointer());
}

void org_blueberry_core_runtime_Activator::stop(ctkPluginContext* context)
{
  m_PrefServiceReg.unregister();
  m_PreferencesService->ShutDown();
  m_PreferencesService = 0;
  m_PrefServiceReg = 0;
}

}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_blueberry_core_runtime, berry::org_blueberry_core_runtime_Activator)
#endif
