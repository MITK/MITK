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

#include "berryCTKPluginActivator.h"

#include "berrySystemBundle.h"
#include "berryInternalPlatform.h"
#include "berryCTKPluginListener_p.h"
#include "berryPreferencesService.h"
#include "berryExtensionPointService.h"

#include <ctkPluginFrameworkLauncher.h>

#include <QCoreApplication>
#include <QtPlugin>

#include <QDebug>

namespace berry {

ctkPluginContext* org_blueberry_core_runtime_Activator::context = 0;

void org_blueberry_core_runtime_Activator::start(ctkPluginContext* context)
{
  this->context = context;

  SystemBundle::Pointer systemBundle = InternalPlatform::GetInstance()->GetBundle("system.bundle").Cast<SystemBundle>();
  m_ExtensionPointService = new ExtensionPointService(&systemBundle->GetBundleLoader());

  // register the service in the legacy BlueBerry service registry
  Platform::GetServiceRegistry().RegisterService(IExtensionPointService::SERVICE_ID, m_ExtensionPointService);

  // register the service in the CTK service registry
  context->registerService<IExtensionPointService>(m_ExtensionPointService.GetPointer());

  m_PreferencesService = new PreferencesService(context->getDataFile("").absolutePath());
  m_PrefServiceReg = context->registerService<IPreferencesService>(m_PreferencesService.GetPointer());

  // register a listener to catch new plugin installations/resolutions.
  pluginListener.reset(new CTKPluginListener(m_ExtensionPointService));
  context->connectPluginListener(pluginListener.data(), SLOT(pluginChanged(ctkPluginEvent)), Qt::DirectConnection);

  // populate the registry with all the currently installed plugins.
  // There is a small window here while processPlugins is being
  // called where the pluginListener may receive a ctkPluginEvent
  // to add/remove a plugin from the registry.  This is ok since
  // the registry is a synchronized object and will not add the
  // same bundle twice.
  pluginListener->processPlugins(context->getPlugins());
}

void org_blueberry_core_runtime_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)

  pluginListener.reset();

  Platform::GetServiceRegistry().UnRegisterService(IExtensionPointService::SERVICE_ID);

  m_PrefServiceReg.unregister();
  m_PreferencesService->ShutDown();
  m_PreferencesService = 0;
  m_PrefServiceReg = 0;

  this->context = 0;
}

ctkPluginContext* org_blueberry_core_runtime_Activator::getPluginContext()
{
  return context;
}

org_blueberry_core_runtime_Activator::~org_blueberry_core_runtime_Activator()
{
}

}

Q_EXPORT_PLUGIN2(org_blueberry_core_runtime, berry::org_blueberry_core_runtime_Activator)
