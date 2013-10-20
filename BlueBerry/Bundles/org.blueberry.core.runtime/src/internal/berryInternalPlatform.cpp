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

#include <Poco/Exception.h>
#include <Poco/File.h>
#include <Poco/FileStream.h>
#include <Poco/AutoPtr.h>
#include <Poco/Util/PropertyFileConfiguration.h>
#include <Poco/StringTokenizer.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/OptionException.h>

#include <ctkPluginFrameworkLauncher.h>
#include <ctkPluginFrameworkFactory.h>
#include <ctkPluginFramework.h>
#include <ctkPluginContext.h>
#include <ctkPlugin.h>
#include <ctkPluginException.h>

#include <iostream>

#include "berryPlatform.h"
#include "berryPlatformException.h"
#include "berryDebugUtil.h"
//#include "event/berryPlatformEvents.h"
//#include "berryPlatformLogChannel.h"
#include "berryProvisioningInfo.h"
#include "berryCTKPluginActivator.h"
#include "berryLogImpl.h"

#include <berryIPreferencesService.h>
#include <berryIExtensionRegistry.h>

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDebug>

namespace berry {

Poco::Mutex InternalPlatform::m_Mutex;

InternalPlatform::InternalPlatform()
  : m_Initialized(false)
  , m_Running(false)
  , m_ConsoleLog(false)
  , m_PlatformLogger(0)
  , m_ctkPluginFrameworkFactory(0)
{
}

InternalPlatform::~InternalPlatform()
{

}

InternalPlatform* InternalPlatform::GetInstance()
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  static InternalPlatform instance;
  return &instance;
}

bool InternalPlatform::ConsoleLog() const
{
  return m_ConsoleLog;
}

ctkPluginContext* InternalPlatform::GetCTKPluginFrameworkContext() const
{
  if (m_ctkPluginFrameworkFactory)
  {
    return m_ctkPluginFrameworkFactory->getFramework()->getPluginContext();
  }
  return 0;
}

IAdapterManager* InternalPlatform::GetAdapterManager() const
{
  AssertInitialized();
  return NULL;
}

void InternalPlatform::Initialize(int& argc, char** argv, Poco::Util::AbstractConfiguration* config)
{
  // initialization
  Poco::Mutex::ScopedLock lock(m_Mutex);

  m_Argc = &argc;
  m_Argv = argv;

  try
  {
    this->init(argc, argv);
  }
  catch (const Poco::Util::UnknownOptionException& e)
  {
    BERRY_WARN << e.displayText();
  }
  this->loadConfiguration();
  if (config)
  {
    this->config().add(config, 50, false);
  }

  m_ConsoleLog = this->GetConfiguration().hasProperty(Platform::ARG_CONSOLELOG.toStdString());

  m_ConfigPath.setPath(QString::fromStdString(this->GetConfiguration().getString("application.configDir")));
  m_InstancePath.setPath(QString::fromStdString(this->GetConfiguration().getString("application.dir")));
  try
  {
    m_InstallPath.setPath(QString::fromStdString(this->GetConfiguration().getString(Platform::ARG_HOME.toStdString())));
  }
  catch (Poco::NotFoundException& )
  {
    m_InstallPath = m_InstancePath;
  }

  if (this->GetConfiguration().hasProperty(Platform::ARG_STORAGE_DIR.toStdString()))
  {
    QString dataLocation = QString::fromStdString(this->GetConfiguration().getString(Platform::ARG_STORAGE_DIR.toStdString(), ""));
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + '_';
#else
    QString dataLocation = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + '_';
#endif
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

  // Initialize the CTK Plugin Framework
  ctkProperties fwProps;
  fwProps.insert(ctkPluginConstants::FRAMEWORK_STORAGE, userFile.absoluteFilePath());
  if (this->GetConfiguration().hasProperty(Platform::ARG_CLEAN.toStdString()))
  {
    fwProps.insert(ctkPluginConstants::FRAMEWORK_STORAGE_CLEAN, ctkPluginConstants::FRAMEWORK_STORAGE_CLEAN_ONFIRSTINIT);
  }
  if (this->GetConfiguration().hasProperty(Platform::ARG_CONSOLELOG.toStdString()))
  {
    fwProps.insert("org.commontk.pluginfw.debug.framework", true);
    fwProps.insert("org.commontk.pluginfw.debug.errors", true);
    fwProps.insert("org.commontk.pluginfw.debug.pluginfw", true);
    fwProps.insert("org.commontk.pluginfw.debug.lazy_activation", true);
    fwProps.insert("org.commontk.pluginfw.debug.resolve", true);
  }
  if (this->GetConfiguration().hasProperty(Platform::ARG_PRELOAD_LIBRARY.toStdString()))
  {
    QString preloadLibs = QString::fromStdString(this->GetConfiguration().getString(Platform::ARG_PRELOAD_LIBRARY.toStdString()));
    fwProps.insert(ctkPluginConstants::FRAMEWORK_PRELOAD_LIBRARIES, preloadLibs.split(',', QString::SkipEmptyParts));
  }
  m_ctkPluginFrameworkFactory = new ctkPluginFrameworkFactory(fwProps);
  QSharedPointer<ctkPluginFramework> pfw = m_ctkPluginFrameworkFactory->getFramework();
  pfw->init();
  ctkPluginContext* pfwContext = pfw->getPluginContext();

  // FIXME: This is a quick-fix for Bug 16224 - Umlaut and other special characters in install/binary path
  // Assumption : linux provides utf8, windows provides ascii encoded argv lists
#ifdef Q_OS_WIN
  QString provisioningFile = QString::fromStdString(this->GetConfiguration().getString(Platform::ARG_PROVISIONING.toStdString()));
#else
  QString provisioningFile = QString::fromUtf8(this->GetConfiguration().getString(Platform::ARG_PROVISIONING.toStdString()).c_str());
#endif
  if (!provisioningFile.isEmpty())
  {
    BERRY_INFO(m_ConsoleLog) << "Using provisioning file: " << provisioningFile;
    ProvisioningInfo provInfo(provisioningFile);
    // it can still happen, that the encoding is not compatible with the fromUtf8 function ( i.e. when manipulating the LANG variable
    // in such case, the QStringList in provInfo is empty which we can easily check for
    if( provInfo.getPluginDirs().empty() )
    {
      BERRY_ERROR << "Cannot search for provisioning file, the retrieved directory list is empty.\n" <<
                     "This can occur if there are some special (non-ascii) characters in the install path.";
      throw berry::PlatformException("No provisioning file specified. Terminating...");
    }
    foreach(QString pluginPath, provInfo.getPluginDirs())
    {
      ctkPluginFrameworkLauncher::addSearchPath(pluginPath);
    }

    bool forcePluginOverwrite = this->GetConfiguration().hasOption(Platform::ARG_FORCE_PLUGIN_INSTALL.toStdString());
    QList<QUrl> pluginsToStart = provInfo.getPluginsToStart();
    foreach(QUrl pluginUrl, provInfo.getPluginsToInstall())
    {
      if (forcePluginOverwrite)
      {
        uninstallPugin(pluginUrl, pfwContext);
      }
      try
      {
        BERRY_INFO(m_ConsoleLog) << "Installing CTK plug-in from: " << pluginUrl.toString().toStdString();
        QSharedPointer<ctkPlugin> plugin = pfwContext->installPlugin(pluginUrl);
        if (pluginsToStart.contains(pluginUrl))
        {
          m_CTKPluginsToStart << plugin->getPluginId();
        }
      }
      catch (const ctkPluginException& e)
      {
        QString errorMsg;
        QDebug dbg(&errorMsg);
        dbg << e.printStackTrace();
        BERRY_ERROR << qPrintable(errorMsg);
      }
    }
  }
  else
  {
    BERRY_INFO << "No provisioning file set.";
  }

  m_BaseStatePath.setPath(m_UserPath.absolutePath() + "/bb-metadata/bb-plugins");

  QString logPath(m_UserPath.absoluteFilePath(QString::fromStdString(this->commandName()) + ".log"));
  m_PlatformLogChannel = new Poco::SimpleFileChannel(logPath.toStdString());
  m_PlatformLogger = &Poco::Logger::create("PlatformLogger", m_PlatformLogChannel, Poco::Message::PRIO_TRACE);

  m_Initialized = true;

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
  DebugUtil::RestoreState(m_UserPath);
#endif

}

void InternalPlatform::uninstallPugin(const QUrl& pluginUrl, ctkPluginContext* pfwContext)
{
  QFileInfo libInfo(pluginUrl.toLocalFile());
  QString libName = libInfo.baseName();
  if (libName.startsWith("lib"))
  {
    libName = libName.mid(3);
  }
  QString symbolicName = libName.replace('_', '.');

  foreach(QSharedPointer<ctkPlugin> plugin, pfwContext->getPlugins())
  {
    if (plugin->getSymbolicName() == symbolicName &&
        plugin->getLocation() != pluginUrl.toString())
    {
      BERRY_WARN << "A plug-in with the symbolic name " << symbolicName.toStdString() <<
                    " but different location is already installed. Trying to uninstall " << plugin->getLocation().toStdString();
      plugin->uninstall();
      return;
    }
  }
}

void InternalPlatform::Launch()
{
  AssertInitialized();

  if (m_Running) return;

  m_Running = true;

  this->run();
}

void InternalPlatform::Shutdown()
{
  QSharedPointer<ctkPluginFramework> ctkPluginFW;

  {
    Poco::Mutex::ScopedLock lock(m_Mutex);
    AssertInitialized();
    DebugUtil::SaveState(m_UserPath);
    ctkPluginFW = m_ctkPluginFrameworkFactory->getFramework();
    m_Initialized = false;
  }

  ctkPluginFW->stop();

  this->uninitialize();

  // wait 10 seconds for the CTK plugin framework to stop
  ctkPluginFW->waitForStop(10000);
}


void InternalPlatform::AssertInitialized() const
{
  if (!m_Initialized)
    throw Poco::SystemException("The Platform has not been initialized yet!");
}

IExtensionRegistry* InternalPlatform::GetExtensionRegistry()
{
  if (m_RegistryTracker.isNull())
  {
    m_RegistryTracker.reset(new ctkServiceTracker<berry::IExtensionRegistry*>(berry::CTKPluginActivator::getPluginContext()));
    m_RegistryTracker->open();
  }
  return m_RegistryTracker->getService();
}

IPreferencesService *InternalPlatform::GetPreferencesService()
{
  if (m_PreferencesTracker.isNull())
  {
    m_PreferencesTracker.reset(new ctkServiceTracker<berry::IPreferencesService*>(berry::CTKPluginActivator::getPluginContext()));
    m_PreferencesTracker->open();
  }
  return m_PreferencesTracker->getService();
}

QDir InternalPlatform::GetConfigurationPath()
{
  return m_ConfigPath;
}

QDir InternalPlatform::GetInstallPath()
{
  return m_InstallPath;
}

QDir InternalPlatform::GetInstancePath()
{
  return m_InstancePath;
}

bool InternalPlatform::GetStatePath(QDir& statePath, const QSharedPointer<ctkPlugin>& plugin, bool create)
{
  QFileInfo tmpStatePath(m_BaseStatePath.absoluteFilePath(plugin->getSymbolicName()));
  if (tmpStatePath.exists())
  {
    if (tmpStatePath.isDir() && tmpStatePath.isWritable() && tmpStatePath.isReadable())
    {
      statePath.setPath(tmpStatePath.absoluteFilePath());
      return true;
    }
    else
    {
      return false;
    }
  }
  else if (create)
  {
    bool created = statePath.mkpath(tmpStatePath.absoluteFilePath());
    if (created)
    {
      statePath.setPath(tmpStatePath.absoluteFilePath());
      return true;
    }
    return false;
  }
  return false;
}

//PlatformEvents& InternalPlatform::GetEvents()
//{
//  return m_Events;
//}

QDir InternalPlatform::GetUserPath()
{
  return m_UserPath;
}

ILog *InternalPlatform::GetLog(const QSharedPointer<ctkPlugin> &plugin) const
{
  LogImpl* result = m_Logs.value(plugin->getPluginId());
  if (result != NULL)
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
  Poco::Mutex::ScopedLock lock(m_Mutex);
  return (m_Initialized && m_Running);
}

QSharedPointer<ctkPlugin> InternalPlatform::GetPlugin(const QString &symbolicName)
{
  QList<QSharedPointer<ctkPlugin> > plugins =
      InternalPlatform::GetInstance()->GetCTKPluginFrameworkContext()->getPlugins();

  QSharedPointer<ctkPlugin> res(0);
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
  QList<QSharedPointer<ctkPlugin> > plugins =
      InternalPlatform::GetInstance()->GetCTKPluginFrameworkContext()->getPlugins();

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

Poco::Logger* InternalPlatform::GetLogger()
{
  return m_PlatformLogger;
}

Poco::Util::LayeredConfiguration& InternalPlatform::GetConfiguration() const
{
  return this->config();
}

QStringList InternalPlatform::GetApplicationArgs() const
{
  return m_FilteredArgs;
}

int& InternalPlatform::GetRawApplicationArgs(char**& argv)
{
  argv = m_Argv;
  return *m_Argc;
}

void InternalPlatform::defineOptions(Poco::Util::OptionSet& options)
{
  Poco::Util::Option helpOption("help", "h", "print this help text");
  helpOption.callback(Poco::Util::OptionCallback<InternalPlatform>(this, &InternalPlatform::PrintHelp));
  options.addOption(helpOption);

  Poco::Util::Option newInstanceOption(Platform::ARG_NEWINSTANCE.toStdString(), "", "forces a new instance of this application");
  newInstanceOption.binding(Platform::ARG_NEWINSTANCE.toStdString());
  options.addOption(newInstanceOption);

  Poco::Util::Option cleanOption(Platform::ARG_CLEAN.toStdString(), "", "cleans the plugin cache");
  cleanOption.binding(Platform::ARG_CLEAN.toStdString());
  options.addOption(cleanOption);

  Poco::Util::Option appOption(Platform::ARG_APPLICATION.toStdString(), "", "the id of the application extension to be executed");
  appOption.argument("<id>").binding(Platform::ARG_APPLICATION.toStdString());
  options.addOption(appOption);

  Poco::Util::Option storageDirOption(Platform::ARG_STORAGE_DIR.toStdString(), "", "the location for storing persistent application data");
  storageDirOption.argument("<dir>").binding(Platform::ARG_STORAGE_DIR.toStdString());
  options.addOption(storageDirOption);

  Poco::Util::Option consoleLogOption(Platform::ARG_CONSOLELOG.toStdString(), "", "log messages to the console");
  consoleLogOption.binding(Platform::ARG_CONSOLELOG.toStdString());
  options.addOption(consoleLogOption);

  Poco::Util::Option forcePluginOption(Platform::ARG_FORCE_PLUGIN_INSTALL.toStdString(), "", "force installing plug-ins with same symbolic name");
  forcePluginOption.binding(Platform::ARG_FORCE_PLUGIN_INSTALL.toStdString());
  options.addOption(forcePluginOption);

  Poco::Util::Option preloadLibsOption(Platform::ARG_PRELOAD_LIBRARY.toStdString(), "", "preload a library");
  preloadLibsOption.argument("<library>").repeatable(true).callback(Poco::Util::OptionCallback<InternalPlatform>(this, &InternalPlatform::handlePreloadLibraryOption));
  options.addOption(preloadLibsOption);

  Poco::Util::Option testPluginOption(Platform::ARG_TESTPLUGIN.toStdString(), "", "the plug-in to be tested");
  testPluginOption.argument("<id>").binding(Platform::ARG_TESTPLUGIN.toStdString());
  options.addOption(testPluginOption);

  Poco::Util::Option testAppOption(Platform::ARG_TESTAPPLICATION.toStdString(), "", "the application to be tested");
  testAppOption.argument("<id>").binding(Platform::ARG_TESTAPPLICATION.toStdString());
  options.addOption(testAppOption);

  Poco::Util::Option noRegistryCacheOption(Platform::ARG_NO_REGISTRY_CACHE.toStdString(), "", "do not use a cache for the registry");
  noRegistryCacheOption.binding(Platform::ARG_NO_REGISTRY_CACHE.toStdString());
  options.addOption(noRegistryCacheOption);

  Poco::Util::Option noLazyRegistryCacheLoadingOption(Platform::ARG_NO_LAZY_REGISTRY_CACHE_LOADING.toStdString(), "", "do not use lazy cache loading for the registry");
  noLazyRegistryCacheLoadingOption.binding(Platform::ARG_NO_LAZY_REGISTRY_CACHE_LOADING.toStdString());
  options.addOption(noLazyRegistryCacheLoadingOption);

  Poco::Util::Option registryMultiLanguageOption(Platform::ARG_REGISTRY_MULTI_LANGUAGE.toStdString(), "", "enable multi-language support for the registry");
  registryMultiLanguageOption.binding(Platform::ARG_REGISTRY_MULTI_LANGUAGE.toStdString());
  options.addOption(registryMultiLanguageOption);

  Poco::Util::Option xargsOption(Platform::ARG_XARGS.toStdString(), "", "Extended argument list");
  xargsOption.argument("<args>").binding(Platform::ARG_XARGS.toStdString());
  options.addOption(xargsOption);

  Poco::Util::Application::defineOptions(options);
}

void InternalPlatform::handlePreloadLibraryOption(const std::string& /*name*/, const std::string& value)
{
  std::string oldVal;
  if (this->config().hasProperty(Platform::ARG_PRELOAD_LIBRARY.toStdString()))
  {
    oldVal = this->config().getString(Platform::ARG_PRELOAD_LIBRARY.toStdString());
  }
  this->config().setString(Platform::ARG_PRELOAD_LIBRARY.toStdString(), oldVal + "," + value);
}

int InternalPlatform::main(const std::vector<std::string>& args)
{
  for(std::vector<std::string>::const_iterator i = args.begin(); i != args.end(); ++i)
  {
    m_FilteredArgs << QString::fromStdString(*i);
  }
  //m_FilteredArgs.insert(m_FilteredArgs.begin(), this->config().getString("application.argv[0]"));

  ctkPluginContext* context = GetCTKPluginFrameworkContext();
  QFileInfo storageDir = context->getDataFile("");

  m_ctkPluginFrameworkFactory->getFramework()->start();
  foreach(long pluginId, m_CTKPluginsToStart)
  {
    BERRY_INFO(m_ConsoleLog) << "Starting CTK plug-in: " << context->getPlugin(pluginId)->getSymbolicName().toStdString()
                             << " [" << pluginId << "]";
    // do not change the autostart setting of this plugin
    context->getPlugin(pluginId)->start(ctkPlugin::START_TRANSIENT | ctkPlugin::START_ACTIVATION_POLICY);
  }

  return EXIT_OK;
}

void InternalPlatform::PrintHelp(const std::string&, const std::string&)
{
  Poco::Util::HelpFormatter help(this->options());
  help.setAutoIndent();
  help.setCommand(this->commandName());
  help.format(std::cout);

  exit(EXIT_OK);
}

}
