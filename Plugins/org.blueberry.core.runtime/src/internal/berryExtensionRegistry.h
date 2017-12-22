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


#ifndef BERRYEXTENSIONREGISTRY_H
#define BERRYEXTENSIONREGISTRY_H

#include <berryIExtensionRegistry.h>

#include <berryIRegistryEventListener.h>
#include "berryRegistryTimestamp.h"
#include "berryCombinedEventDelta.h"
#include "berryListenerList.h"

#include <QReadWriteLock>
#include <QWaitCondition>

class QTranslator;

namespace berry {

struct IContributor;
struct IStatus;

class ConfigurationElementDescription;
class RegistryContribution;
class RegistryContributor;
class RegistryObject;
class RegistryObjectFactory;
class RegistryObjectManager;
class RegistryStrategy;

/**
 * An implementation for the extension registry API.
 */
class ExtensionRegistry : public QObject, public IExtensionRegistry
{
  Q_OBJECT
  Q_INTERFACES(berry::IExtensionRegistry)

private:

  struct ListenerInfo;

  class RegistryEventThread;

  // used to enforce concurrent access policy for readers/writers
  mutable QReadWriteLock access;

  // all registry event listeners
  ListenerList<ListenerInfo> listeners;

  QScopedPointer<IRegistryEventListener> debugRegistryListener;

  SmartPointer<RegistryObjectManager> registryObjects;

  QObject* masterToken; // use to get full control of the registry; objects created as "static"
  QObject* userToken; // use to modify non-persisted registry elements

  RegistryTimestamp aggregatedTimestamp; // tracks current contents of the registry

  // encapsulates processing of new registry deltas
  CombinedEventDelta eventDelta;
  // marks a new extended delta. The namespace that normally would not exists is used for this purpose
  const static QString notNamespace;

  // does this instance of the extension registry has multiple language support enabled?
  bool isMultiLanguage;

  // have we already logged a error on usage of an unsupported multi-language method?
  mutable bool mlErrorLogged;

  QScopedPointer<RegistryEventThread> eventThread; // registry event loop

  // The pair of values we store in the event queue
  struct QueueElement;

  class Queue : public QList<QueueElement>
  {
    QMutex mutex;
    QWaitCondition waitCond;

  public:

    class Locker {
    private: Queue* q;
    public:
      Locker(Queue* q) : q(q) { q->mutex.lock(); }
      ~Locker() { q->mutex.unlock(); }
    };

    void wait() { waitCond.wait(&mutex); }
    void notify() { waitCond.wakeOne(); }
  };
  Queue queue; // stores registry events info

  /**
   * Adds and resolves all extensions and extension points provided by the
   * plug-in.
   * <p>
   * A corresponding IRegistryChangeEvent will be broadcast to all listeners
   * interested on changes in the given plug-in.
   * </p>
   */
  void Add(const SmartPointer<RegistryContribution>& element);

  QString AddExtension(int extension);

  /**
   * Looks for existing orphan extensions to connect to the given extension
   * point. If none is found, there is nothing to do. Otherwise, link them.
   */
  QString AddExtensionPoint(int extPoint);

  QSet<QString> AddExtensionsAndExtensionPoints(const SmartPointer<RegistryContribution>& element);

  void AddListenerInternal(IRegistryEventListener* listener, const IExtensionPointFilter& filter);

  void BasicAdd(const SmartPointer<RegistryContribution>& element, bool link);

  void SetObjectManagers(const SmartPointer<IObjectManager> &manager);

  void BasicRemove(const QString& contributorId);

  /**
   * Broadcasts (asynchronously) the event to all interested parties.
   */
  void FireRegistryChangeEvent();

  //RegistryDelta GetDelta(const QString& namespaze) const;

  void Link(const SmartPointer<ExtensionPoint>& extPoint, const QList<int>& extensions);

  /*
   * Records an extension addition/removal.
   */
  //QString RecordChange(const SmartPointer<ExtensionPoint>& extPoint, int extension, int kind);

  /*
   * Records a set of extension additions/removals.
   */
  //QString RecordChange(const SmartPointer<ExtensionPoint>& extPoint, const QList<int>& extensions, int kind);

  //Return the affected namespace
  QString RemoveExtension(int extensionId);

  QString RemoveExtensionPoint(int extPoint);

  QSet<QString> RemoveExtensionsAndExtensionPoints(const QString& contributorId);

  /**
   * Access check for add/remove operations:
   * - Master key allows all operations
   * - User key allows modifications of non-persisted elements
   *
   * @param key key to the registry supplied by the user
   * @param persist true if operation affects persisted elements
   * @return true is the key grants read/write access to the registry
   */
  bool CheckReadWriteAccess(QObject* key, bool persist) const;

  void LogError(const QString& owner, const QString& contributionName, const ctkException& e);
  void LogError(const QString& owner, const QString& contributionName);

  // Fill in the actual content of this extension
  void CreateExtensionData(const QString& contributorId,
    const ConfigurationElementDescription& description,
    const SmartPointer<RegistryObject>& parent, bool persist);

  bool RemoveObject(const SmartPointer<RegistryObject>& registryObject,
                    bool isExtensionPoint, QObject* token);

protected:

  //storage manager associated with the registry cache
  //StorageManager cacheStorageManager;

  // Table reader associated with this extension registry
  //TableReader theTableReader = new TableReader(this);

  QScopedPointer<RegistryStrategy> strategy; // overridable portions of the registry functionality

  /**
   * Sets new cache file manager. If existing file manager was owned by the registry,
   * closes it.
   *
   * @param cacheBase the base location for the registry cache
   * @param isCacheReadOnly whether the file cache is read only
   */
  void SetFileManager(const QString& cacheBase, bool isCacheReadOnly);

  // allow other objects in the registry to use the same lock
  void EnterRead();

  // allow other objects in the registry to use the same lock
  void ExitRead();

  /////////////////////////////////////////////////////////////////////////////////////////////////
  // Registry Object Factory
  // The factory produces contributions, extension points, extensions, and configuration elements
  // to be stored in the extension registry.
  QScopedPointer<RegistryObjectFactory> theRegistryObjectFactory;

  // Override to provide domain-specific elements to be stored in the extension registry
  void SetElementFactory();

  //TableReader getTableReader() const;

  // Find the first location that contains a cache table file and set file manager to it.
  bool CheckCache();

  void StopChangeEventScheduler();

public:

  SmartPointer<RegistryObjectManager> GetObjectManager() const;

  void AddListener(IRegistryEventListener* listener, const QString& extensionPointId = QString()) override;
  void AddListener(IRegistryEventListener *listener, const IExtensionPointFilter& filter) override;

  /*
   * @see IExtensionRegistry#getConfigurationElementsFor(java.lang.  QString)
   */
  QList<SmartPointer<IConfigurationElement> > GetConfigurationElementsFor(const QString& extensionPointId) const override;

  /*
   * @see org.eclipse.core.runtime.IExtensionRegistry#getConfigurationElementsFor(java.lang.  QString, java.lang.  QString)
   */
  QList<SmartPointer<IConfigurationElement> > GetConfigurationElementsFor(const QString& pluginId,
                                                                          const QString& extensionPointSimpleId) const override;

  /*
   * @see org.eclipse.core.runtime.IExtensionRegistry#getConfigurationElementsFor(java.lang.  QString, java.lang.  QString, java.lang.  QString)
   */
  QList<SmartPointer<IConfigurationElement> > GetConfigurationElementsFor(const QString& pluginId,
                                                                          const QString& extensionPointName,
                                                                          const QString& extensionId) const override;

  /*
   * @see org.eclipse.core.runtime.IExtensionRegistry#getExtension(java.lang.  QString)
   */
  SmartPointer<IExtension> GetExtension(const QString& extensionId) const override;

  /*
   * @see org.eclipse.core.runtime.IExtensionRegistry#getExtension(java.lang.  QString, java.lang.  QString)
   */
  SmartPointer<IExtension> GetExtension(const QString& extensionPointId, const QString& extensionId) const override;

  /*
   * @see org.eclipse.core.runtime.IExtensionRegistry#getExtension(java.lang.  QString, java.lang.  QString, java.lang.  QString)
   */
  SmartPointer<IExtension> GetExtension(const QString& pluginId,
                                        const QString& extensionPointName,
                                        const QString& extensionId) const override;

  /*
   * @see org.eclipse.core.runtime.IExtensionRegistry#getExtensionPoint(java.lang.  QString)
   */
  SmartPointer<IExtensionPoint> GetExtensionPoint(const QString& xptUniqueId) const override;

  /*
   * @see org.eclipse.core.runtime.IExtensionRegistry#getExtensionPoint(java.lang.  QString, java.lang.  QString)
   */
  SmartPointer<IExtensionPoint> GetExtensionPoint(const QString& elementName, const QString& xpt) const override;

  /*
   * @see org.eclipse.core.runtime.IExtensionRegistry#getExtensionPoints()
   */
  QList<SmartPointer<IExtensionPoint> > GetExtensionPoints() const override;

  /*
   * @see org.eclipse.core.runtime.IExtensionRegistry#getExtensionPoints(java.lang.  QString)
   */
  QList<SmartPointer<IExtensionPoint> > GetExtensionPoints(const QString& namespaceName) const override;

  /*
   * @see org.eclipse.core.runtime.IExtensionRegistry#getExtensions(java.lang.  QString)
   */
  QList<SmartPointer<IExtension> > GetExtensions(const QString& namespaceName) const override;

  QList<SmartPointer<IExtension> > GetExtensions(const SmartPointer<IContributor>& contributor) const override;

  QList<SmartPointer<IExtensionPoint> > GetExtensionPoints(const SmartPointer<IContributor>& contributor) const override;

  /*
   * @see org.eclipse.core.runtime.IExtensionRegistry#getNamespaces()
   */
  QList<QString> GetNamespaces() const override;

  bool HasContributor(const SmartPointer<IContributor>& contributor) const;

  bool HasContributor(const QString& contributorId) const;

  void Remove(const QString& removedContributorId, long timestamp);

  void RemoveContributor(const SmartPointer<IContributor>& contributor, QObject* key);

  /**
   * Unresolves and removes all extensions and extension points provided by
   * the plug-in.
   * <p>
   * A corresponding IRegistryChangeEvent will be broadcast to all listeners
   * interested on changes in the given plug-in.
   * </p>
   */
  void Remove(const QString& removedContributorId);

  void RemoveListener(IRegistryEventListener* listener) override;

  ExtensionRegistry(RegistryStrategy* registryStrategy, QObject* masterToken, QObject* userToken);

  ~ExtensionRegistry() override;

  /**
   * Stops the registry. Registry has to be stopped to properly
   * close cache and dispose of listeners.
   * @param key - key token for this registry
   */
  void Stop(QObject* key) override;

  /*
   * Clear the registry cache files from the file manager so on next start-up we recompute it.
   */
  void ClearRegistryCache();

  // Lazy initialization.
  RegistryObjectFactory* GetElementFactory();

  void Log(const SmartPointer<IStatus>& status) const;

  /**
   * With multi-locale support enabled this method returns the non-translated
   * key so that they can be cached and translated later into desired languages.
   * In the absence of the multi-locale support the key gets translated immediately
   * and only translated values is cached.
   */
  QString Translate(const QString& key, QTranslator* resources) const;

  bool Debug() const;

  bool DebugEvents() const;

  bool UseLazyCacheLoading() const;

  long ComputeState() const;

  QObject* CreateExecutableExtension(const SmartPointer<RegistryContributor>& defaultContributor,
                                     const QString& className, const QString& requestedContributorName);

  //////////////////////////////////////////////////////////////////////////////////////////
  // Registry change events processing

  void ProcessChangeEvent(const QList<ListenerInfo>& listenerInfos,
                                           const CombinedEventDelta &scheduledDelta);

  // Registry events notifications are done on a separate thread in a sequential manner
  // (first in - first processed)
  void ScheduleChangeEvent(const QList<ListenerInfo>& listenerInfos,
                           const CombinedEventDelta& scheduledDeltas);

  bool AddContribution(QIODevice* is, const SmartPointer<IContributor>& contributor,
                       bool persist, const QString& contributionName,
                       QTranslator* translationBundle, QObject* key, long timestamp);

  bool AddContribution(QIODevice* is, const SmartPointer<IContributor>& contributor, bool persist,
                       const QString& contributionName, QTranslator* translationBundle, QObject* key) override;

  /**
   * Adds an extension point to the extension registry.
   * <p>
   * If the registry is not modifiable, this method is an access controlled method.
   * Proper token should be passed as an argument for non-modifiable registries.
   * </p>
   * @param identifier Id of the extension point. If non-qualified names is supplied,
   * it will be converted internally into a fully qualified name
   * @param contributor the contributor of this extension point
   * @param persist indicates if contribution should be stored in the registry cache. If false,
   * contribution is not persisted in the registry cache and is lost on Eclipse restart
   * @param label display string for the extension point
   * @param schemaReference reference to the extension point schema. The schema reference
   * is a URL path relative to the plug-in installation URL. May be null
   * @param token the key used to check permissions. Two registry keys are set in the registry
   * constructor {@link RegistryFactory#createRegistry(org.eclipse.core.runtime.spi.RegistryStrategy, Object, Object)}:
   * master token and a user token. Master token allows all operations; user token
   * allows non-persisted registry elements to be modified.
   * @return <code>true</code> if successful, <code>false</code> if a problem was encountered
   * @throws IllegalArgumentException if incorrect token is passed in
   */
  bool AddExtensionPoint(const QString& identifier, const SmartPointer<IContributor>& contributor,
                         bool persist, const QString& label, const QString& schemaReference, QObject* token);

  /**
   * Adds an extension to the extension registry.
   * <p>
   * If the registry is not modifiable, this method is an access controlled method.
   * Proper token should be passed as an argument for non-modifiable registries.
   * </p>
   * @see org.eclipse.core.internal.registry.spi.ConfigurationElementDescription
   *
   * @param identifier Id of the extension. If non-qualified name is supplied,
   * it will be converted internally into a fully qualified name
   * @param contributor the contributor of this extension
   * @param persist indicates if contribution should be stored in the registry cache. If false,
   * contribution is not persisted in the registry cache and is lost on Eclipse restart
   * @param label display string for this extension
   * @param extensionPointId Id of the point being extended. If non-qualified
   * name is supplied, it is assumed to have the same contributorId as this extension
   * @param configurationElements contents of the extension
   * @param token the key used to check permissions. Two registry keys are set in the registry
   * constructor {@link RegistryFactory#createRegistry(org.eclipse.core.runtime.spi.RegistryStrategy, Object, Object)}:
   * master token and a user token. Master token allows all operations; user token
   * allows non-persisted registry elements to be modified.
   * @return <code>true</code> if successful, <code>false</code> if a problem was encountered
   * @throws IllegalArgumentException if incorrect token is passed in
   */
  bool AddExtension(const QString& identifier, const SmartPointer<IContributor>& contributor,
                    bool persist, const QString& label, const QString& extensionPointId,
                    const ConfigurationElementDescription& configurationElements, QObject* token);

  bool RemoveExtension(const SmartPointer<IExtension>& extension, QObject* token) override;

  bool RemoveExtensionPoint(const SmartPointer<IExtensionPoint>& extensionPoint, QObject* token) override;

  QList<SmartPointer<IContributor> > GetAllContributors() const;

  bool IsMultiLanguage() const override;

  QList<QString> Translate(const QList<QString>& nonTranslated, const SmartPointer<IContributor>& contributor,
                           const QLocale& locale) const;

  QLocale GetLocale() const;

  void LogMultiLangError() const;

};

}

#endif // BERRYEXTENSIONREGISTRY_H
