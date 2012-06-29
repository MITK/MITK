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

#include "berryPlatform.h"
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

#if defined(Q_OS_LINUX) || defined(Q_OS_DARWIN) || defined(Q_CC_MINGW)

#include <dlfcn.h>
QString org_blueberry_core_runtime_Activator::getPluginId(void *symbol)
{
  if (symbol == NULL) return QString();

  Dl_info info = {0,0,0,0};
  if(dladdr(symbol, &info) == 0)
  {
    return QString();
  }
  else if(info.dli_fname)
  {
    QFile soPath(info.dli_fname);
    int index = soPath.fileName().lastIndexOf('.');
    QString pluginId = soPath.fileName().left(index);
    if (pluginId.startsWith("lib"))
      pluginId = pluginId.mid(3);
    return pluginId.replace('_', '.');
  }
  return QString();
}

#elif defined(Q_CC_MSVC)

#include <ctkBackTrace.h>
#include <windows.h>
#include <dbghelp.h>
QString org_blueberry_core_runtime_Activator::getPluginId(void *symbol)
{
  if (symbol == NULL) return QString();

  if (ctk::DebugSymInitialize())
  {
    std::vector<char> moduleBuffer(sizeof(IMAGEHLP_MODULE64));
    PIMAGEHLP_MODULE64 pModuleInfo = (PIMAGEHLP_MODULE64)&moduleBuffer.front();
    pModuleInfo->SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
    if (SymGetModuleInfo64(GetCurrentProcess(), (DWORD64)symbol, pModuleInfo))
    {
      QString pluginId = pModuleInfo->ModuleName;
      return pluginId.replace('_', '.');
    }
  }
  return QString();
}

#endif

org_blueberry_core_runtime_Activator::~org_blueberry_core_runtime_Activator()
{
}

}

Q_EXPORT_PLUGIN2(org_blueberry_core_runtime, berry::org_blueberry_core_runtime_Activator)
