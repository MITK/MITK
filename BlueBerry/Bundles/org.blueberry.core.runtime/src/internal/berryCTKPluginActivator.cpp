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

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "berryCTKPluginActivator.h"

#include "berryPlatform.h"
#include "berryInternalPlatform.h"
//#include "berryCTKPluginListener_p.h"
#include "berryPreferencesService.h"
#include "berryExtensionRegistry.h"
#include "berryRegistryConstants.h"
#include "berryRegistryProperties.h"
#include "berryRegistryStrategy.h"

#include <QCoreApplication>
#include <QtPlugin>

#include <QDebug>

namespace berry {

ctkPluginContext* org_blueberry_core_runtime_Activator::context = 0;

void org_blueberry_core_runtime_Activator::start(ctkPluginContext* context)
{
  this->context = context;

  RegistryProperties::SetContext(context);
  //ProcessCommandLine();
  this->startRegistry();

  preferencesService.reset(new PreferencesService(context->getDataFile("").absolutePath()));
  prefServiceReg = context->registerService<IPreferencesService>(preferencesService.data());

//  // register a listener to catch new plugin installations/resolutions.
//  pluginListener.reset(new CTKPluginListener(m_ExtensionPointService));
//  context->connectPluginListener(pluginListener.data(), SLOT(pluginChanged(ctkPluginEvent)), Qt::DirectConnection);

//  // populate the registry with all the currently installed plugins.
//  // There is a small window here while processPlugins is being
//  // called where the pluginListener may receive a ctkPluginEvent
//  // to add/remove a plugin from the registry.  This is ok since
//  // the registry is a synchronized object and will not add the
//  // same bundle twice.
//  pluginListener->processPlugins(context->getPlugins());
}

void org_blueberry_core_runtime_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)

  //pluginListener.reset();

  //Platform::GetServiceRegistry().UnRegisterService(IExtensionPointService::SERVICE_ID);

  prefServiceReg.unregister();
  preferencesService->ShutDown();
  preferencesService.reset();
  prefServiceReg = 0;

  this->stopRegistry();
  RegistryProperties::SetContext(NULL);

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

org_blueberry_core_runtime_Activator::org_blueberry_core_runtime_Activator()
  : userRegistryKey(new QObject())
  , masterRegistryKey(new QObject())
{
}

org_blueberry_core_runtime_Activator::~org_blueberry_core_runtime_Activator()
{
}

void org_blueberry_core_runtime_Activator::startRegistry()
{
  // see if the customer suppressed the creation of default registry
  QString property = context->getProperty(RegistryConstants::PROP_DEFAULT_REGISTRY).toString();
  if (property.compare("false", Qt::CaseInsensitive) == 0) return;

  // check to see if we need to use null as a userToken
  if (context->getProperty(RegistryConstants::PROP_REGISTRY_NULL_USER_TOKEN).toString().compare("true", Qt::CaseInsensitive) == 0)
  {
    userRegistryKey.reset(0);
  }

  // Determine primary and alternative registry locations. BlueBerry extension registry cache
  // can be found in one of the two locations:
  // a) in the local configuration area (standard location passed in by the platform) -> priority
  // b) in the shared configuration area (typically, shared install is used)
  QList<QString> registryLocations;
  QList<bool> readOnlyLocations;

  RegistryStrategy* strategy = NULL;
  //Location configuration = OSGIUtils.getDefault().getConfigurationLocation();
  QString configuration = context->getDataFile("").absoluteFilePath();
  if (configuration.isEmpty())
  {
    RegistryProperties::SetProperty(RegistryConstants::PROP_NO_REGISTRY_CACHE, "true");
    RegistryProperties::SetProperty(RegistryConstants::PROP_NO_LAZY_CACHE_LOADING, "true");
    strategy = new RegistryStrategy(QList<QString>(), QList<bool>(), masterRegistryKey.data());
  }
  else
  {
    //File primaryDir = new File(configuration.getURL().getPath() + '/' + STORAGE_DIR);
    //bool primaryReadOnly = configuration.isReadOnly();
    QString primaryDir = configuration;
    bool primaryReadOnly = false;

    //Location parentLocation = configuration.getParentLocation();
    QString parentLocation;
    if (!parentLocation.isEmpty())
    {
//      File secondaryDir = new File(parentLocation.getURL().getFile() + '/' + IRegistryConstants.RUNTIME_NAME);
//      registryLocations << primaryDir << secondaryDir;
//      readOnlyLocations << primaryReadOnly <<  true; // secondary BlueBerry location is always read only
    }
    else
    {
      registryLocations << primaryDir;
      readOnlyLocations << primaryReadOnly;
    }
    strategy = new RegistryStrategy(registryLocations, readOnlyLocations, masterRegistryKey.data());
  }

  ExtensionRegistry* registry = new ExtensionRegistry(strategy, masterRegistryKey.data(), userRegistryKey.data());
  defaultRegistry.reset(registry);

  registryServiceReg = context->registerService<IExtensionRegistry>(registry);
  //commandRegistration = EquinoxUtils.registerCommandProvider(Activator.getContext());
}

void org_blueberry_core_runtime_Activator::stopRegistry()
{
  if (!defaultRegistry.isNull())
  {
    registryServiceReg.unregister();
    defaultRegistry->Stop(masterRegistryKey.data());
  }
//  if (!commandRegistration.isNull())
//  {
//    commandRegistration.unregister();
//  }
}

}

#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
  Q_EXPORT_PLUGIN2(org_blueberry_core_runtime, berry::org_blueberry_core_runtime_Activator)
#endif
