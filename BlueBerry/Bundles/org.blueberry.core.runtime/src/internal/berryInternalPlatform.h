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

#include <ctkServiceTracker.h>

#include <QDir>

#include <map>

class ctkPluginFrameworkFactory;
class ctkPluginContext;

namespace berry {

struct IAdapterManager;
struct IExtensionRegistry;
struct IPreferencesService;
struct ILog;

class LogImpl;
class PlatformLogChannel;

class InternalPlatform : private Poco::Util::Application
{
private:

  static Poco::Mutex m_Mutex;

  bool m_Initialized;
  bool m_Running;

  bool m_ConsoleLog;

  QScopedPointer<ctkServiceTracker<berry::IPreferencesService*> > m_PreferencesTracker;
  QScopedPointer<ctkServiceTracker<berry::IExtensionRegistry*> > m_RegistryTracker;

  QDir m_BaseStatePath;
  QDir m_InstallPath;
  QDir m_InstancePath;
  QDir m_UserPath;
  QDir m_ConfigPath;

  QStringList m_FilteredArgs;

  Poco::AutoPtr<Poco::SimpleFileChannel> m_PlatformLogChannel;
  Poco::Logger* m_PlatformLogger;

  mutable QHash<int, LogImpl*> m_Logs;

  ctkPluginFrameworkFactory* m_ctkPluginFrameworkFactory;
  QList<long> m_CTKPluginsToStart;

  //PlatformEvents m_Events;
  //PlatformEvent m_EventStarted;

  int* m_Argc;
  char** m_Argv;

  //std::map<QString, QString> m_ArgMap;

  InternalPlatform();
  //InternalPlatform(const InternalPlatform&) : m_EventStarted(PlatformEvent::EV_PLATFORM_STARTED) {};

  void AssertInitialized() const;

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

  IAdapterManager* GetAdapterManager() const;

  IExtensionRegistry* GetExtensionRegistry();

  IPreferencesService* GetPreferencesService();

  bool ConsoleLog() const;

  QDir GetConfigurationPath();

  QDir GetInstallPath();

  QDir GetInstancePath();

  bool GetStatePath(QDir& statePath, const QSharedPointer<ctkPlugin>& plugin, bool create = true);

  QDir GetUserPath();

  /**
   * Returns a log for the given plugin. Creates a new one if needed.
   * XXX change this into a LogMgr service that would keep track of the map. See if it can be a service factory.
   * It would contain all the logging methods that are here.
   * Relate to RuntimeLog if appropriate.
   * The system log listener needs to be optional: turned on or off. What about a system property? :-)
   */
  ILog* GetLog(const QSharedPointer<ctkPlugin>& plugin) const;

  //PlatformEvents& GetEvents();

  bool IsRunning() const;

  Poco::Util::LayeredConfiguration& GetConfiguration() const;

  QStringList GetApplicationArgs() const;

  int& GetRawApplicationArgs(char**& argv);

  QSharedPointer<ctkPlugin> GetPlugin(const QString& symbolicName);

  QList<QSharedPointer<ctkPlugin> > GetPlugins(const QString& symbolicName,
                                               const QString& version);

  Poco::Logger* GetLogger();

};

}  // namespace berry

#endif /*BERRYINTERNALPLATFORM_H_*/
