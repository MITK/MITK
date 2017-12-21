/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYAPPLICATIONCONTAINER_H
#define BERRYAPPLICATIONCONTAINER_H

#include <berryIRegistryEventListener.h>

#include <ctkServiceTracker.h>
#include <ctkServiceTrackerCustomizer.h>

#include <QObject>
#include <QMutex>

struct ctkApplicationLauncher;
struct ctkApplicationRunnable;

namespace berry {

struct IBranding;
struct IExtensionRegistry;

class ApplicationDescriptor;
class ApplicationHandle;
class MainApplicationLauncher;

/*
 * This container will discover installed BlueBerry applications and register the
 * appropriate ctkApplicationDescriptor service with the service registry.
 */
class ApplicationContainer : public QObject, public QMutex, private IRegistryEventListener,
    private ctkServiceTrackerCustomizer<ctkApplicationLauncher*>
{
  Q_OBJECT

private:

  static const QString PI_RUNTIME; // = "org.blueberry.core.runtime";
  static const QString PT_APPLICATIONS; // = "applications";
  static const QString PT_APP_VISIBLE; // = "visible";
  static const QString PT_APP_THREAD; // = "thread";
  static const QString PT_APP_THREAD_ANY; // = "any";
  static const QString PT_APP_CARDINALITY; // = "cardinality";
  static const QString PT_APP_CARDINALITY_SINGLETON_GLOBAL; // = "singleton-global";
  static const QString PT_APP_CARDINALITY_SINGLETON_SCOPED; // = "singleton-scoped";
  static const QString PT_APP_CARDINALITY_UNLIMITED; // = "*";
  static const QString PT_APP_ICON; // = "icon";
  static const QString PT_PRODUCTS; // = "products";
  static const QString EXT_ERROR_APP; // = "org.blueberry.core.runtime.app.error";

  static const QString PROP_PRODUCT; // = "blueberry.product";
  static const QString PROP_BLUEBERRY_APPLICATION; // = "blueberry.application";
  static const QString PROP_BLUEBERRY_APPLICATION_LAUNCH_DEFAULT; // = "blueberry.application.launchDefault";

  static const int NOT_LOCKED; // = 0;
  static const int LOCKED_SINGLETON_GLOBAL_RUNNING; // = 1;
  static const int LOCKED_SINGLETON_GLOBAL_APPS_RUNNING; // = 2;
  static const int LOCKED_SINGLETON_SCOPED_RUNNING; // = 3;
  static const int LOCKED_SINGLETON_LIMITED_RUNNING; // = 4;
  static const int LOCKED_MAIN_THREAD_RUNNING; // = 5;

  ctkPluginContext* context;

  QMutex lock;

  // A map of ApplicationDescriptors keyed by application ID
  /* @GuardedBy(lock) */
  QHash<QString, ApplicationDescriptor*> apps;

  IExtensionRegistry* extensionRegistry;
  QScopedPointer<ctkServiceTracker<ctkApplicationLauncher*>> launcherTracker;
  mutable QScopedPointer<IBranding> branding;
  mutable bool missingProductReported;

  /* @GuardedBy(lock) */
  QList<ApplicationHandle*> activeHandles; // the currently active application handles
  /* @GuardedBy(lock) */
  ApplicationHandle* activeMain; // the handle currently running on the main thread
  /* @GuardedBy(lock) */
  ApplicationHandle* activeGlobalSingleton; // the current global singleton handle
  /* @GuardedBy(lock) */
  ApplicationHandle* activeScopedSingleton; // the current scoped singleton handle
  /* @GuardedBy(lock) */
  QHash<QString, QList<ApplicationHandle*> > activeLimited; // Map of handles that have cardinality limits

  mutable QString defaultAppId;
  //DefaultApplicationListener defaultAppListener;
  ctkApplicationRunnable* defaultMainThreadAppHandle; // holds the default app handle to be run on the main thread
  volatile bool missingApp;
  QScopedPointer<MainApplicationLauncher> missingAppLauncher;

public:

  ApplicationContainer(ctkPluginContext* context, IExtensionRegistry* extensionRegistry);
  ~ApplicationContainer() override;

  void Start();
  void Stop();

  IBranding* GetBranding() const;

  ctkPluginContext* GetContext() const;

  void Launch(ApplicationHandle* appHandle);

  void Lock(ApplicationHandle* appHandle);
  void Unlock(ApplicationHandle* appHandle);
  int IsLocked(const ApplicationDescriptor* eclipseApp) const;

  void StartDefaultApp(bool delayError);

private:

  Q_DISABLE_COPY(ApplicationContainer)

  friend class ApplicationHandle;

  /*
   * Only used to find the default application
   */
  ApplicationDescriptor* GetAppDescriptor(const QString& applicationId);

  ApplicationDescriptor* CreateAppDescriptor(const SmartPointer<IExtension>& appExtension);

  ApplicationDescriptor* RemoveAppDescriptor(const QString& applicationId);

  /*
   * Registers an ApplicationDescriptor service for each eclipse application
   * available in the extension registry.
   */
  void RegisterAppDescriptors();

  void RegisterAppDescriptor(const QString& applicationId);

  /*
   * Returns a list of all the available application IDs which are available
   * in the extension registry.
   */
  QList<SmartPointer<IExtension> > GetAvailableAppExtensions() const;

  QString GetAvailableAppsMsg() const;

  /*
   * Returns the application extension for the specified applicaiton ID.
   * A RuntimeException is thrown if the extension does not exist for the
   * given application ID.
   */
  SmartPointer<IExtension> GetAppExtension(const QString& applicationId) const;

  Q_SLOT void PluginChanged(const ctkPluginEvent& event);

  void StopAllApps();

  QString GetDefaultAppId() const;

  ctkApplicationLauncher* addingService(const ctkServiceReference& reference) override;
  void modifiedService(const ctkServiceReference& reference, ctkApplicationLauncher* service) override;
  void removedService(const ctkServiceReference& reference, ctkApplicationLauncher* service) override;

  void Added(const QList<SmartPointer<IExtension> >& extensions) override;
  void Added(const QList<SmartPointer<IExtensionPoint> >& extensionPoints) override;
  void Removed(const QList<SmartPointer<IExtension> >& extensions) override;
  void Removed(const QList<SmartPointer<IExtensionPoint> >& extensionPoints) override;

  void RefreshAppDescriptors();

};

}

#endif // BERRYAPPLICATIONCONTAINER_H
