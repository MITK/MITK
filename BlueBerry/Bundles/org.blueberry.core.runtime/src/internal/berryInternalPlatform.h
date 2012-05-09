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

#ifndef BERRYINTERNALPLATFORM_H_
#define BERRYINTERNALPLATFORM_H_

#include <Poco/Mutex.h>
#include <Poco/AutoPtr.h>
#include <Poco/Logger.h>
#include <Poco/SimpleFileChannel.h>
#include <Poco/Util/Application.h>

//#include "event/berryPlatformEvents.h"
#include "service/berryServiceRegistry.h"
//#include "berryExtensionPointService.h"

#include <berryIBundle.h>

#include <ctkServiceTracker.h>

#include <QDir>

#include <map>

class ctkPluginFrameworkFactory;
class ctkPluginContext;

namespace berry {

struct IBundle;
struct IExtensionPointService;
struct IExtensionRegistry;
struct IPreferencesService;

class CodeCache;
class BundleLoader;
class PlatformLogChannel;
class SystemBundle;

class org_blueberry_core_runtime_EXPORT InternalPlatform : private Poco::Util::Application
{
private:

  static Poco::Mutex m_Mutex;

  bool m_Initialized;
  bool m_Running;

  bool m_ConsoleLog;

  ServiceRegistry* m_ServiceRegistry;

  QScopedPointer<ctkServiceTracker<berry::IPreferencesService*> > m_PreferencesTracker;

  QDir m_BaseStatePath;
  QDir m_InstallPath;
  QDir m_InstancePath;
  QDir m_UserPath;
  QDir m_ConfigPath;

  QStringList m_FilteredArgs;

  CodeCache* m_CodeCache;
  BundleLoader* m_BundleLoader;

  SystemBundle* m_SystemBundle;

  Poco::AutoPtr<Poco::SimpleFileChannel> m_PlatformLogChannel;
  Poco::Logger* m_PlatformLogger;

  ctkPluginFrameworkFactory* m_ctkPluginFrameworkFactory;
  QList<long> m_CTKPluginsToStart;

  //PlatformEvents m_Events;
  //PlatformEvent m_EventStarted;

  int* m_Argc;
  char** m_Argv;

  //std::map<QString, QString> m_ArgMap;

  InternalPlatform();
  //InternalPlatform(const InternalPlatform&) : m_EventStarted(PlatformEvent::EV_PLATFORM_STARTED) {};

  void AssertInitialized();

  void handlePreloadLibraryOption(const std::string &name, const std::string &value);

  int main(const std::vector<std::string>& args);

  void uninstallPugin(const QUrl& pluginUrl, ctkPluginContext* pfwContext);

public:
  virtual ~InternalPlatform();

  // Poco::Application method overrides
  void defineOptions(Poco::Util::OptionSet& options);

  void PrintHelp(const std::string& name, const std::string& value);

  static InternalPlatform* GetInstance();

  void Initialize(int& argc, char** argv, Poco::Util::AbstractConfiguration* config = 0);
  void Launch();
  void Shutdown();

  ctkPluginContext* GetCTKPluginFrameworkContext() const;

  /// Returns a ServiceRegistry object for registering
  /// and accessing services from different plugins
  ServiceRegistry& GetServiceRegistry();

  /// Convenience method to quickly get the extension
  /// point service, which is automatically started
  /// by the platform
  IExtensionPointService* GetExtensionPointService();

  IExtensionRegistry* GetExtensionRegistry();

  IPreferencesService* GetPreferencesService();

  bool ConsoleLog() const;

  QDir GetConfigurationPath();

  QDir GetInstallPath();

  QDir GetInstancePath();

  bool GetStatePath(QDir& statePath, const QSharedPointer<ctkPlugin>& plugin, bool create = true);

  bool GetStatePath(QDir& statePath, const SmartPointer<IBundle>& bundle, bool create = true);

  QDir GetUserPath();

  //PlatformEvents& GetEvents();

  bool IsRunning() const;

  Poco::Util::LayeredConfiguration& GetConfiguration() const;

  QStringList GetApplicationArgs() const;

  int& GetRawApplicationArgs(char**& argv);

  IBundle::Pointer GetBundle(const QString& id);

  std::vector<IBundle::Pointer> GetBundles() const;

  QSharedPointer<ctkPlugin> GetPlugin(const QString& symbolicName);

  QList<QSharedPointer<ctkPlugin> > GetPlugins(const QString& symbolicName,
                                               const QString& version);

  Poco::Logger* GetLogger();

};

}  // namespace berry

#endif /*BERRYINTERNALPLATFORM_H_*/
