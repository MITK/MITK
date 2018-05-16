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

#ifndef BERRYREGISTRYOBJECTMANAGER_H
#define BERRYREGISTRYOBJECTMANAGER_H

#include "berryIObjectManager.h"
#include "berryHashtableOfStringAndInt.h"
#include "berryKeyedHashSet.h"

#include <QMutex>

namespace berry {

class ExtensionHandle;
class ExtensionPoint;
class ExtensionPointHandle;
class ExtensionRegistry;
class RegistryContribution;
class RegistryContributor;
class RegistryIndexElement;
class RegistryObject;
class RegistryObjectReferenceMap;

/**
 * This class manage all the object from the registry but does not deal with their dependencies.
 * It serves the objects which are either directly obtained from memory or read from a cache.
 * It also returns handles for objects.
 */
class RegistryObjectManager : public IObjectManager
{

public:

  berryObjectMacro(berry::RegistryObjectManager)

  //Constants used to get the objects and their handles
  enum Type {
    CONFIGURATION_ELEMENT = 1,
    EXTENSION,
    EXTENSION_POINT,
    THIRDLEVEL_CONFIGURATION_ELEMENT
  };

  RegistryObjectManager(ExtensionRegistry* registry);
  ~RegistryObjectManager() override;

  void Add(const SmartPointer<RegistryObject>& registryObject, bool hold);

  SmartPointer<RegistryObject> GetObject(int id, short type) const override;

  // The current impementation of this method assumes that we don't cache dynamic
  // extension. In this case all extensions not yet loaded (i.e. not in the memory cache)
  // are "not dynamic" and we actually check memory objects to see if they are dynamic.
  //
  // If we decide to allow caching of dynamic objects, the implementation
  // of this method would have to retrieved the object from disk and check
  // its "dynamic" status. The problem is that id alone is not enough to get the object
  // from the disk; object type is needed as well.
  bool ShouldPersist(int id) const;

  QList<SmartPointer<RegistryObject> > GetObjects(const QList<int>& values, short type) const override;

  SmartPointer<Handle> GetHandle(int id, short type) const override;

  QList<SmartPointer<Handle> > GetHandles(const QList<int>& ids, short type) const override;

  bool IsDirty() const;

  void MarkDirty();

  void Close() override;

  ExtensionRegistry* GetRegistry() const;

  QList<SmartPointer<ExtensionHandle> > GetExtensionsFromContributor(const QString& contributorId) const;

  QList<SmartPointer<ExtensionPointHandle> > GetExtensionPointsFromContributor(const QString& contributorId) const;

  QList<SmartPointer<ExtensionPointHandle> > GetExtensionPointsFromNamespace(const QString& namespaceName) const;

  // This method filters out extensions with no extension point
  QList<SmartPointer<ExtensionHandle> > GetExtensionsFromNamespace(const QString& namespaceName) const;

  SmartPointer<RegistryContributor> GetContributor(const QString& id) const;

protected:

  friend class Extension;
  friend class ExtensionPoint;
  friend class ExtensionRegistry;
  friend class ExtensionsParser;
  friend class RegistryContribution;
  friend class RegistryObject;

  mutable QMutex mutex;

  static const int CACHE_INITIAL_SIZE; // = 512;

  static int UNKNOWN;

  typedef QHash<QString, QList<int> > OrphansMapType;
  typedef QHash<QString, SmartPointer<RegistryContributor> > ContributorsMapType;

  /**
   * Initialize the object manager. Return true if the initialization succeeded, false otherwise
   */
  bool Init(long timeStamp);

  void AddContribution(const SmartPointer<RegistryContribution>& contribution);

  QList<int> GetExtensionPointsFrom(const QString& id) const;
  QList<int> GetExtensionPointsFrom_unlocked(const QString& id) const;

  bool HasContribution(const QString& id) const;

  void Remove(int id, bool release);
  void Remove_unlocked(int id, bool release);

  QList<SmartPointer<RegistryObject> > GetObjects_unlocked(const QList<int>& values, short type) const;

  SmartPointer<ExtensionPoint> GetExtensionPointObject(const QString& xptUniqueId) const;
  SmartPointer<ExtensionPoint> GetExtensionPointObject_unlocked(const QString& xptUniqueId) const;

  QList<SmartPointer<ExtensionPointHandle> > GetExtensionPointsHandles() const;

  SmartPointer<ExtensionPointHandle> GetExtensionPointHandle(const QString& xptUniqueId);

  QList<int> GetExtensionsFrom(const QString& contributorId) const;
  QList<int> GetExtensionsFrom_unlocked(const QString& contributorId) const;

  bool AddExtensionPoint(const SmartPointer<ExtensionPoint>& currentExtPoint, bool hold);

  void RemoveExtensionPoint(const QString& extensionPointId);

  void RemoveContribution(const QString &contributorId);

  void AddOrphans(const QString& extensionPoint, const QList<int>& extensions);

  void AddOrphan(const QString& extensionPoint, int extension);

  QList<int> RemoveOrphans(const QString& extensionPoint);

  void RemoveOrphan(const QString& extensionPoint, int extension);

  //This method is only used by the writer to reach in
  QHash<QString, QList<int> > GetOrphanExtensions() const;

  //  This method is only used by the writer to reach in
  int GetNextId() const;

  //  This method is only used by the writer to reach in
  HashtableOfStringAndInt GetExtensionPoints() const;

  //  This method is only used by the writer to reach in
  QList<KeyedHashSet> GetContributions() const;

  // This method is used internally and by the writer to reach in. Notice that it doesn't
  // return contributors marked as removed.
  ContributorsMapType& GetContributors() const;

  // only adds a contributor if it is not already present in the table
  void AddContributor(const SmartPointer<RegistryContributor>& newContributor);

  void RemoveContributor(const QString& id);

  KeyedHashSet& GetNamespacesIndex() const;

  /**
   * Collect all the objects that are removed by this operation and store
   * them in a IObjectManager so that they can be accessed from the appropriate
   * deltas but not from the registry.
   */
  QHash<int, SmartPointer<RegistryObject> > GetAssociatedObjects(const QString& contributionId) const;

  /**
   * Adds elements to be removed along with the registry object.
   */
  void AddAssociatedObjects(QHash<int, SmartPointer<RegistryObject> >& map,
                            const SmartPointer<RegistryObject>& registryObject);

  /**
   * Add to the set of the objects all extensions and extension points that
   * could be navigated to from the objects in the set.
   */
  void AddNavigableObjects(QHash<int, SmartPointer<RegistryObject> >& associatedObjects);

  void RemoveObjects(const QHash<int, SmartPointer<RegistryObject> >& associatedObjects);

  SmartPointer<IObjectManager> CreateDelegatingObjectManager(const QHash<int, SmartPointer<RegistryObject> >& object);

  bool UnlinkChildFromContributions(int id);

private:

  // key: extensionPointName, value: object id
  HashtableOfStringAndInt extensionPoints; //This is loaded on startup. Then entries can be added when loading a new plugin from the xml.
  // key: object id, value: an object
  RegistryObjectReferenceMap* cache; //Entries are added by getter. The structure is not thread safe.
  //key: int, value: int
  //OffsetTable fileOffsets = null; //This is read once on startup when loading from the cache. Entries are never added here. They are only removed to prevent "removed" objects to be reloaded.

  int nextId; //This is only used to get the next number available.

  mutable bool orphanExtensionsLoaded;

  //Those two data structures are only used when the addition or the removal of a plugin occurs.
  //They are used to keep track on a contributor basis of the extension being added or removed
  KeyedHashSet newContributions; //represents the contributers added during this session.
  mutable KeyedHashSet formerContributions; //represents the contributers encountered in previous sessions. This is loaded lazily.
  mutable bool formerContributionsLoaded;

  mutable QHash<QString, SmartPointer<RegistryContributor> > contributors; // key: contributor ID; value: contributor name
  mutable bool contributorsLoaded;
  QHash<QString, SmartPointer<RegistryContributor> > removedContributors; // key: contributor ID; value: contributor name
  mutable KeyedHashSet namespacesIndex; // registry elements (extension & extensionpoints) indexed by namespaces
  mutable bool namespacesIndexLoaded;

  // Map key: extensionPointFullyQualifiedName, value int[] of orphan extensions.
  // The orphan access does not need to be synchronized because the it is protected by the lock in extension registry.
  mutable QHash<QString, QList<int> > orphanExtensions;

  KeyedHashSet heldObjects; //strong reference to the objects that must be hold on to

  //Indicate if objects have been removed or added from the table. This only needs to be set in a couple of places (addNamespace and removeNamespace)
  bool isDirty;

  bool fromCache;

  ExtensionRegistry* registry;


  // TODO make ExtensionPoint, Extension provide namespace in a same way (move it to the RegistryObject?)
  // See if all the registryObjects have the same namespace. If not, return null.
  // Also can return null if empty array is passed in or objects are of an unexpected type
  QString FindCommonNamespaceIdentifier(const QList<SmartPointer<RegistryObject> >& registryObjects) const;

  void RemoveExtensionPointFromNamespaceIndex(int extensionPoint, const QString& namespaceName);

  void RemoveExtensionFromNamespaceIndex(int extensions, const QString& namespaceName);

  // Called from a synchronized method
  void UpdateNamespaceIndex(const SmartPointer<RegistryContribution>& contribution, bool added);

  KeyedHashSet& GetFormerContributions() const;

  void Remove(const SmartPointer<RegistryObject>& registryObject, bool release);

  void Hold(const SmartPointer<RegistryObject>& toHold);

  void Release(const SmartPointer<RegistryObject>& toRelease);

  SmartPointer<RegistryObject> BasicGetObject(int id, short type) const;

  SmartPointer<RegistryObject> Load(int id, short type) const;

  OrphansMapType& GetOrphans() const;

  // Find or create required index element
  SmartPointer<RegistryIndexElement> GetNamespaceIndex(const QString& namespaceName) const;

  void CollectChildren(const SmartPointer<RegistryObject>& ce, int level,
                       QHash<int, SmartPointer<RegistryObject> >& collector) const;

  // Called from a synchronized method only
  bool UnlinkChildFromContributions(const QList<SmartPointer<KeyedElement> >& contributions, int id);

};

}

#endif // BERRYREGISTRYOBJECTMANAGER_H
