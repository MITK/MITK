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

#include "berryInternalPlatform.h"

#include "berryLog.h"
#include "berryLogImpl.h"
#include "berryPlatform.h"
#include "berryPlatformException.h"
#include "berryDebugUtil.h"
#include "berryPlatformException.h"
#include "berryCTKPluginActivator.h"
#include "berryPlatformException.h"
#include "berryApplicationContainer.h"
#include "berryProduct.h"

#include "berryIBranding.h"

//#include "event/berryPlatformEvents.h"
//#include "berryPlatformLogChannel.h"

#include <berryIApplicationContext.h>
#include <berryIPreferencesService.h>
#include <berryIExtensionRegistry.h>
#include <berryIProduct.h>

#include <service/datalocation/ctkLocation.h>
#include <service/debug/ctkDebugOptions.h>

#include <ctkPluginContext.h>
#include <ctkPlugin.h>
#include <ctkPluginException.h>
#include <ctkPluginFrameworkLauncher.h>

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDebug>
#include <QMutexLocker>

namespace berry {

QMutex InternalPlatform::m_Mutex;

bool InternalPlatform::DEBUG = false;
bool InternalPlatform::DEBUG_PLUGIN_PREFERENCES = false;

InternalPlatform::InternalPlatform()
  : m_Initialized(false)
  , m_ConsoleLog(false)
  , m_Context(nullptr)
{
}

InternalPlatform::~InternalPlatform()
{

}

InternalPlatform* InternalPlatform::GetInstance()
{
  QMutexLocker lock(&m_Mutex);
  static InternalPlatform instance;
  return &instance;
}

bool InternalPlatform::ConsoleLog() const
{
  return m_ConsoleLog;
}

QVariant InternalPlatform::GetOption(const QString& option, const QVariant& defaultValue) const
{
  ctkDebugOptions* options = GetDebugOptions();
  if (options != nullptr)
  {
    return options->getOption(option, defaultValue);
  }
  return QVariant();
}

IAdapterManager* InternalPlatform::GetAdapterManager() const
{
  AssertInitialized();
  return nullptr;
}

SmartPointer<IProduct> InternalPlatform::GetProduct() const
{
  if (product.IsNotNull()) return product;
  ApplicationContainer* container = org_blueberry_core_runtime_Activator::GetContainer();
  IBranding* branding = container == nullptr ? nullptr : container->GetBranding();
  if (branding == nullptr) return IProduct::Pointer();
  IProduct::Pointer brandingProduct = branding->GetProduct();
  if (!brandingProduct)
  {
    brandingProduct = new Product(branding);
  }
  product = brandingProduct;
  return product;
}

void InternalPlatform::InitializePluginPaths()
{
  QMutexLocker lock(&m_Mutex);

  // Add search paths for Qt plugins
  foreach(QString qtPluginPath, m_Context->getProperty(Platform::PROP_QTPLUGIN_PATH).toStringList())
  {
    if (QFile::exists(qtPluginPath))
    {
      QCoreApplication::addLibraryPath(qtPluginPath);
    }
    else if (m_ConsoleLog)
    {
      BERRY_WARN << "Qt plugin path does not exist: " << qtPluginPath.toStdString();
    }
  }

  // Add a default search path. It is assumed that installed applications
  // provide their Qt plugins in that path.
  static const QString defaultQtPluginPath = QCoreApplication::applicationDirPath() + "/plugins";
  if (QFile::exists(defaultQtPluginPath))
  {
    QCoreApplication::addLibraryPath(defaultQtPluginPath);
  }

  if (m_ConsoleLog)
  {
    std::string pathList;
    foreach(QString libPath, QCoreApplication::libraryPaths())
    {
      pathList += (pathList.empty() ? "" : ", ") + libPath.toStdString();
    }
    BERRY_INFO << "Qt library search paths: " << pathList;
  }

  /*
  m_ConfigPath.setPath(m_Context->getProperty("application.configDir").toString());
  m_InstancePath.setPath(m_Context->getProperty("application.dir").toString());
  QString installPath = m_Context->getProperty(Platform::PROP_HOME).toString();
  if (installPath.isEmpty())
  {
    m_InstallPath = m_InstancePath;
  }
  else {
    m_InstallPath.setPath(installPath);
  }

  QString dataLocation = m_Context->getProperty(Platform::PROP_STORAGE_DIR).toString();
  if (!storageDir.isEmpty())
  {
    if (dataLocation.at(dataLocation.size()-1) != '/')
    {
      dataLocation += '/';
    }
    m_UserPath.setPath(dataLocation);
  }
  else
  {
    // Append a hash value of the absolute path of the executable to the data location.
    // This allows to start the same application from different build or install trees.
    dataLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + this->getOrganizationName() + "/" + this->getApplicationName() + '_';
    dataLocation += QString::number(qHash(QCoreApplication::applicationDirPath())) + "/";
    m_UserPath.setPath(dataLocation);
  }
  BERRY_INFO(m_ConsoleLog) << "Framework storage dir: " << m_UserPath.absolutePath();

  QFileInfo userFile(m_UserPath.absolutePath());

  if (!QDir().mkpath(userFile.absoluteFilePath()) || !userFile.isWritable())
  {
    QString tmpPath = QDir::temp().absoluteFilePath(QString::fromStdString(this->commandName()));
    BERRY_WARN << "Storage dir " << userFile.absoluteFilePath() << " is not writable. Falling back to temporary path " << tmpPath;
    QDir().mkpath(tmpPath);
    userFile.setFile(tmpPath);
  }

  m_BaseStatePath.setPath(m_UserPath.absolutePath() + "/bb-metadata/bb-plugins");

  QString logPath(m_UserPath.absoluteFilePath(QString::fromStdString(this->commandName()) + ".log"));
  m_PlatformLogChannel = new Poco::SimpleFileChannel(logPath.toStdString());
*/
}

ctkDebugOptions* InternalPlatform::GetDebugOptions() const
{
  return m_DebugTracker.isNull() ? nullptr : m_DebugTracker->getService();
}

IApplicationContext* InternalPlatform::GetApplicationContext() const
{
  QList<ctkServiceReference> refs;
  try
  {
    refs = m_Context->getServiceReferences<IApplicationContext>("(blueberry.application.type=main.thread)");
  }
  catch (const std::invalid_argument&)
  {
    return nullptr;
  }
  if (refs.isEmpty()) return nullptr;

  // assumes the application context is available as a service
  IApplicationContext* result = m_Context->getService<IApplicationContext>(refs.front());
  if (result != nullptr)
  {
    m_Context->ungetService(refs.front());
    return result;
  }
  return nullptr;
}

void InternalPlatform::Start(ctkPluginContext* context)
{
  this->m_Context = context;

  m_ConsoleLog = m_Context->getProperty(ctkPluginFrameworkLauncher::PROP_CONSOLE_LOG).toBool();

  OpenServiceTrackers();

  this->InitializePluginPaths();

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
  DebugUtil::RestoreState(m_UserPath);
#endif

  InitializeDebugFlags();
  this->m_Initialized = true;
}

void InternalPlatform::Stop(ctkPluginContext* /*context*/)
{
  AssertInitialized();

  this->m_Initialized = false;
  CloseServiceTrackers();

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
  DebugUtil::SaveState(m_UserPath);
#endif

  this->m_Context = nullptr;
}


void InternalPlatform::AssertInitialized() const
{
  if (!m_Initialized)
  {
    throw PlatformException("The Platform has not been initialized yet!");
  }
}

void InternalPlatform::OpenServiceTrackers()
{
  ctkPluginContext* context = this->m_Context;

  instanceLocation.reset(new ctkServiceTracker<ctkLocation*>(context, ctkLDAPSearchFilter(ctkLocation::INSTANCE_FILTER)));
  instanceLocation->open();

  userLocation.reset(new ctkServiceTracker<ctkLocation*>(context, ctkLDAPSearchFilter(ctkLocation::USER_FILTER)));
  userLocation->open();

  configurationLocation.reset(new ctkServiceTracker<ctkLocation*>(context, ctkLDAPSearchFilter(ctkLocation::CONFIGURATION_FILTER)));
  configurationLocation->open();

  installLocation.reset(new ctkServiceTracker<ctkLocation*>(context, ctkLDAPSearchFilter(ctkLocation::INSTALL_FILTER)));
  installLocation->open();

  m_PreferencesTracker.reset(new ctkServiceTracker<berry::IPreferencesService*>(context));
  m_PreferencesTracker->open();

  m_RegistryTracker.reset(new ctkServiceTracker<berry::IExtensionRegistry*>(context));
  m_RegistryTracker->open();

  m_DebugTracker.reset(new ctkServiceTracker<ctkDebugOptions*>(context));
  m_DebugTracker->open();
}

void InternalPlatform::CloseServiceTrackers()
{
  if (!m_PreferencesTracker.isNull())
  {
    m_PreferencesTracker->close();
    m_PreferencesTracker.reset();
  }
  if (!m_RegistryTracker.isNull())
  {
    m_RegistryTracker->close();
    m_RegistryTracker.reset();
  }
  if (!m_DebugTracker.isNull())
  {
    m_DebugTracker->close();
    m_DebugTracker.reset();
  }

  if (!configurationLocation.isNull()) {
    configurationLocation->close();
    configurationLocation.reset();
  }

  if (!installLocation.isNull()) {
    installLocation->close();
    installLocation.reset();
  }

  if (!instanceLocation.isNull()) {
    instanceLocation->close();
    instanceLocation.reset();
  }

  if (!userLocation.isNull()) {
    userLocation->close();
    userLocation.reset();
  }
}

void InternalPlatform::InitializeDebugFlags()
{
  DEBUG = this->GetOption(Platform::PI_RUNTIME + "/debug", false).toBool();
  if (DEBUG)
  {
    DEBUG_PLUGIN_PREFERENCES = GetOption(Platform::PI_RUNTIME + "/preferences/plugin", false).toBool();
  }
}

IExtensionRegistry* InternalPlatform::GetExtensionRegistry()
{
  return m_RegistryTracker.isNull() ? nullptr : m_RegistryTracker->getService();
}

IPreferencesService *InternalPlatform::GetPreferencesService()
{
  return m_PreferencesTracker.isNull() ? nullptr : m_PreferencesTracker->getService();
}

ctkLocation* InternalPlatform::GetConfigurationLocation()
{
  this->AssertInitialized();
  return configurationLocation->getService();
}

ctkLocation* InternalPlatform::GetInstallLocation()
{
  this->AssertInitialized();
  return installLocation->getService();
}

ctkLocation* InternalPlatform::GetInstanceLocation()
{
  this->AssertInitialized();
  return instanceLocation->getService();
}

QDir InternalPlatform::GetStateLocation(const QSharedPointer<ctkPlugin>& plugin)
{
  ctkLocation* service = GetInstanceLocation();
  if (service == nullptr)
  {
    throw ctkIllegalStateException("No instance data can be specified.");
  }

  QUrl url = GetInstanceLocation()->getDataArea(plugin->getSymbolicName());
  if (!url.isValid())
  {
    throw ctkIllegalStateException("The instance data location has not been specified yet.");
  }

  QDir location(url.toLocalFile());
  if (!location.exists())
  {
    if (!location.mkpath(location.absolutePath()))
    {
      throw PlatformException(QString("Could not create plugin state location \"%1\"").arg(location.absolutePath()));
    }
  }
  return location;
}

//PlatformEvents& InternalPlatform::GetEvents()
//{
//  return m_Events;
//}

ctkLocation* InternalPlatform::GetUserLocation()
{
  this->AssertInitialized();
  return userLocation->getService();
}

ILog *InternalPlatform::GetLog(const QSharedPointer<ctkPlugin> &plugin) const
{
  LogImpl* result = m_Logs.value(plugin->getPluginId());
  if (result != nullptr)
    return result;

//  ExtendedLogService logService = (ExtendedLogService) extendedLogTracker.getService();
//  Logger logger = logService == null ? null : logService.getLogger(bundle, PlatformLogWriter.EQUINOX_LOGGER_NAME);
//  result = new Log(bundle, logger);
//  ExtendedLogReaderService logReader = (ExtendedLogReaderService) logReaderTracker.getService();
//  logReader.addLogListener(result, result);
//  logs.put(bundle, result);
//  return result;

  result = new LogImpl(plugin);
  m_Logs.insert(plugin->getPluginId(), result);
  return result;
}

bool InternalPlatform::IsRunning() const
{
  QMutexLocker lock(&m_Mutex);
  try
  {
    return m_Initialized && m_Context && m_Context->getPlugin()->getState() == ctkPlugin::ACTIVE;
  }
  catch (const ctkIllegalStateException&)
  {
    return false;
  }
}

QSharedPointer<ctkPlugin> InternalPlatform::GetPlugin(const QString &symbolicName)
{
  QList<QSharedPointer<ctkPlugin> > plugins = m_Context->getPlugins();

  QSharedPointer<ctkPlugin> res(nullptr);
  foreach(QSharedPointer<ctkPlugin> plugin, plugins)
  {
    if ((plugin->getState() & (ctkPlugin::INSTALLED | ctkPlugin::UNINSTALLED)) == 0 &&
        plugin->getSymbolicName() == symbolicName)
    {
      if (res.isNull())
      {
        res = plugin;
      }
      else if (res->getVersion().compare(plugin->getVersion()) < 0)
      {
        res = plugin;
      }
    }
  }
  return res;
}

QList<QSharedPointer<ctkPlugin> > InternalPlatform::GetPlugins(const QString &symbolicName, const QString &version)
{
  QList<QSharedPointer<ctkPlugin> > plugins = m_Context->getPlugins();

  QMap<ctkVersion, QSharedPointer<ctkPlugin> > selected;
  ctkVersion versionObj(version);
  foreach(QSharedPointer<ctkPlugin> plugin, plugins)
  {
    if ((plugin->getState() & (ctkPlugin::INSTALLED | ctkPlugin::UNINSTALLED)) == 0 &&
        plugin->getSymbolicName() == symbolicName)
    {
      if (plugin->getVersion().compare(versionObj) > -1)
      {
        selected.insert(plugin->getVersion(), plugin);
      }
    }
  }
  QList<QSharedPointer<ctkPlugin> > sortedPlugins = selected.values();
  QList<QSharedPointer<ctkPlugin> > reversePlugins;
  qCopyBackward(sortedPlugins.begin(), sortedPlugins.end(), reversePlugins.end());
  return reversePlugins;
}

QStringList InternalPlatform::GetApplicationArgs() const
{
  QStringList result;

  IApplicationContext* appContext = this->GetApplicationContext();
  if (appContext)
  {
    QHash<QString, QVariant> args = appContext->GetArguments();
    result = args[IApplicationContext::APPLICATION_ARGS].toStringList();
  }
  return result;
}

}
