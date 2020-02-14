/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYINTERNALPLATFORM_H_
#define BERRYINTERNALPLATFORM_H_

#include <berrySmartPointer.h>

#include <ctkServiceTracker.h>

#include <QDir>
#include <QMutex>

struct ctkDebugOptions;
struct ctkLocation;
class ctkPluginFrameworkFactory;
class ctkPluginContext;

namespace berry {

struct IAdapterManager;
struct IApplicationContext;
struct IExtensionRegistry;
struct IPreferencesService;
struct IProduct;
struct ILog;

class LogImpl;
class PlatformLogChannel;

class InternalPlatform
{
private:

  static QMutex m_Mutex;

  bool m_Initialized;
  bool m_Running;

  bool m_ConsoleLog;

  ctkPluginContext* m_Context;

  QScopedPointer<ctkServiceTracker<berry::IPreferencesService*> > m_PreferencesTracker;
  QScopedPointer<ctkServiceTracker<berry::IExtensionRegistry*> > m_RegistryTracker;
  QScopedPointer<ctkServiceTracker<ctkDebugOptions*> > m_DebugTracker;

  QScopedPointer<ctkServiceTracker<ctkLocation*>> configurationLocation;
  QScopedPointer<ctkServiceTracker<ctkLocation*>> installLocation;
  QScopedPointer<ctkServiceTracker<ctkLocation*>> instanceLocation;
  QScopedPointer<ctkServiceTracker<ctkLocation*>> userLocation;

  mutable SmartPointer<IProduct> product;

  mutable QHash<int, LogImpl*> m_Logs;

  //PlatformEvents m_Events;
  //PlatformEvent m_EventStarted;

  //std::map<QString, QString> m_ArgMap;

  InternalPlatform();
  //InternalPlatform(const InternalPlatform&) : m_EventStarted(PlatformEvent::EV_PLATFORM_STARTED) {};

  void AssertInitialized() const;

  void OpenServiceTrackers();
  void CloseServiceTrackers();
  void InitializeDebugFlags();
  void InitializeLocations();

  ctkDebugOptions* GetDebugOptions() const;

  IApplicationContext* GetApplicationContext() const;

  void InitializePluginPaths();

public:

  static bool DEBUG;
  static bool DEBUG_PLUGIN_PREFERENCES;

  virtual ~InternalPlatform();

  static InternalPlatform* GetInstance();

  void Start(ctkPluginContext* context);
  void Stop(ctkPluginContext* context);

  IAdapterManager* GetAdapterManager() const;

  SmartPointer<IProduct> GetProduct() const;


  IExtensionRegistry* GetExtensionRegistry();

  IPreferencesService* GetPreferencesService();

  bool ConsoleLog() const;

  QVariant GetOption(const QString& option, const QVariant& defaultValue = QVariant()) const;

  ctkLocation* GetConfigurationLocation();

  ctkLocation* GetInstallLocation();

  ctkLocation* GetInstanceLocation();

  QDir GetStateLocation(const QSharedPointer<ctkPlugin>& plugin);

  ctkLocation* GetUserLocation();

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

  QStringList GetApplicationArgs() const;

  QSharedPointer<ctkPlugin> GetPlugin(const QString& symbolicName);

  QList<QSharedPointer<ctkPlugin> > GetPlugins(const QString& symbolicName,
                                               const QString& version);

};

}  // namespace berry

#endif /*BERRYINTERNALPLATFORM_H_*/
