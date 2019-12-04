/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "berryRegistryObjectManager.h"

#include "berryConfigurationElementHandle.h"
#include "berryExtension.h"
#include "berryExtensionHandle.h"
#include "berryExtensionPoint.h"
#include "berryExtensionPointHandle.h"
#include "berryInvalidRegistryObjectException.h"
#include "berryRegistryObjectReferenceMap.h"
#include "berryRegistryConstants.h"
#include "berryRegistryContribution.h"
#include "berryRegistryContributor.h"
#include "berryRegistryIndexElement.h"
#include "berryRegistryObject.h"
#include "berryRegistryProperties.h"
#include "berryTemporaryObjectManager.h"
#include "berryThirdLevelConfigurationElementHandle.h"

#include <limits>

namespace berry {

const int RegistryObjectManager::CACHE_INITIAL_SIZE = 512;
int RegistryObjectManager::UNKNOWN = std::numeric_limits<int>::max();

RegistryObjectManager::RegistryObjectManager(ExtensionRegistry* registry)
  : cache(nullptr), nextId(1), orphanExtensionsLoaded(false), formerContributionsLoaded(false),
    contributorsLoaded(false), namespacesIndexLoaded(false),
    isDirty(false), fromCache(false), registry(registry)
{
  if (RegistryProperties::GetProperty(RegistryConstants::PROP_NO_REGISTRY_FLUSHING)
      .compare("true", Qt::CaseInsensitive) == 0)
  {
    cache = new RegistryObjectReferenceMap(RegistryObjectReferenceMap::HARD,
                                           CACHE_INITIAL_SIZE);
  }
  else
  {
    cache = new RegistryObjectReferenceMap(RegistryObjectReferenceMap::SOFT,
                                           CACHE_INITIAL_SIZE);
  }
}

RegistryObjectManager::~RegistryObjectManager()
{
  delete cache;
}

void RegistryObjectManager::Add(const SmartPointer<RegistryObject>& registryObject, bool hold)
{
  if (registryObject->GetObjectId() == UNKNOWN)
  {
    int id = nextId++;
    registryObject->SetObjectId(id);
  }
  cache->Put(registryObject->GetObjectId(), registryObject);
  if (hold)
    Hold(registryObject);
}

SmartPointer<RegistryObject> RegistryObjectManager::GetObject(int id, short type) const
{
  return BasicGetObject(id, type);
}

bool RegistryObjectManager::ShouldPersist(int id) const
{
  RegistryObject::Pointer result = cache->Get(id);
  if (result.IsNotNull())
    return result->ShouldPersist();
  return true;
}

QList<SmartPointer<RegistryObject> > RegistryObjectManager::GetObjects(const QList<int>& values, short type) const
{
  QMutexLocker l(&mutex);
  return this->GetObjects_unlocked(values, type);
}

QList<SmartPointer<RegistryObject> > RegistryObjectManager::GetObjects_unlocked(const QList<int>& values, short type) const
{
  if (values.empty())
  {
    return QList<RegistryObject::Pointer>();
  }

  QList<RegistryObject::Pointer> results;
  for (int i = 0; i < values.size(); ++i)
  {
    results.push_back(BasicGetObject(values[i], type));
  }
  return results;
}

SmartPointer<Handle> RegistryObjectManager::GetHandle(int id, short type) const
{
  Handle::Pointer result;
  switch (type)
  {
  case EXTENSION_POINT :
    result = new ExtensionPointHandle(IObjectManager::ConstPointer(this), id);
    break;
  case EXTENSION :
    result = new ExtensionHandle(IObjectManager::ConstPointer(this), id);
    break;
  case CONFIGURATION_ELEMENT :
    result = new ConfigurationElementHandle(IObjectManager::ConstPointer(this), id);
    break;
  case THIRDLEVEL_CONFIGURATION_ELEMENT :
  default : //avoid compiler error, type should always be known
    result = new ThirdLevelConfigurationElementHandle(IObjectManager::ConstPointer(this), id);
  }
  return result;
}

QList<SmartPointer<Handle> > RegistryObjectManager::GetHandles(const QList<int>& ids, short type) const
{
  QList<Handle::Pointer> results;
  if (ids.empty()) return results;

  switch (type)
  {
  case EXTENSION_POINT :
    for (int i = 0; i < ids.size(); i++)
    {
      Handle::Pointer handle(new ExtensionPointHandle(this, ids[i]));
      results.push_back(handle);
    }
    break;
  case EXTENSION :
    for (int i = 0; i < ids.size(); i++)
    {
      Handle::Pointer handle(new ExtensionHandle(this, ids[i]));
      results.push_back(handle);
    }
    break;
  case CONFIGURATION_ELEMENT :
    for (int i = 0; i < ids.size(); i++)
    {
      Handle::Pointer handle(new ConfigurationElementHandle(this, ids[i]));
      results.push_back(handle);
    }
    break;
  case THIRDLEVEL_CONFIGURATION_ELEMENT :
    for (int i = 0; i < ids.size(); i++)
    {
      Handle::Pointer handle(new ThirdLevelConfigurationElementHandle(this, ids[i]));
      results.push_back(handle);
    }
    break;
  }
  return results;
}

bool RegistryObjectManager::IsDirty() const
{
  return isDirty;
}

void RegistryObjectManager::MarkDirty()
{
  isDirty = true;
}

void RegistryObjectManager::Close()
{
  //do nothing.
}

ExtensionRegistry* RegistryObjectManager::GetRegistry() const
{
  return registry;
}

QList<SmartPointer<ExtensionHandle> >
RegistryObjectManager::GetExtensionsFromContributor(const QString& contributorId) const
{
  QList<int> ids = GetExtensionsFrom(contributorId);
  QList<ExtensionHandle::Pointer> result;
  foreach(Handle::Pointer handle, GetHandles(ids, EXTENSION))
  {
    result.push_back(handle.Cast<ExtensionHandle>());
  }
  return result;
}

QList<SmartPointer<ExtensionPointHandle> >
RegistryObjectManager::GetExtensionPointsFromContributor(const QString& contributorId) const
{
  QList<int> ids = GetExtensionPointsFrom(contributorId);
  QList<ExtensionPointHandle::Pointer> result;
  foreach(Handle::Pointer handle, GetHandles(ids, EXTENSION_POINT))
  {
    result.push_back(handle.Cast<ExtensionPointHandle>());
  }
  return result;
}

QList<SmartPointer<ExtensionPointHandle> >
RegistryObjectManager::GetExtensionPointsFromNamespace(const QString& namespaceName) const
{
  QMutexLocker l(&mutex);
  RegistryIndexElement::Pointer indexElement = GetNamespaceIndex(namespaceName);
  QList<int> namespaceExtensionPoints = indexElement->GetExtensionPoints();
  QList<ExtensionPointHandle::Pointer> result;
  foreach(Handle::Pointer handle, GetHandles(namespaceExtensionPoints, EXTENSION_POINT))
  {
    result.push_back(handle.Cast<ExtensionPointHandle>());
  }
  return result;
}

QList<SmartPointer<ExtensionHandle> >
RegistryObjectManager::GetExtensionsFromNamespace(const QString& namespaceName) const
{
  QMutexLocker l(&mutex);
  RegistryIndexElement::Pointer indexElement = GetNamespaceIndex(namespaceName);
  QList<int> namespaceExtensions = indexElement->GetExtensions();

  // filter extensions with no extension point (orphan extensions)
  QList<ExtensionHandle::Pointer> result;
  QList<RegistryObject::Pointer> exts = GetObjects_unlocked(namespaceExtensions, EXTENSION);
  for (int i = 0; i < exts.size(); i++)
  {
    Extension::Pointer ext = exts[i].Cast<Extension>();
    if (GetExtensionPointObject_unlocked(ext->GetExtensionPointIdentifier()).IsNotNull())
    {
      result.push_back(GetHandle(ext->GetObjectId(), EXTENSION).Cast<ExtensionHandle>());
    }
  }
  return result;
}

bool RegistryObjectManager::Init(long /*timeStamp*/)
{
  return false;
//  QMutexLocker l(&mutex);
//  TableReader reader = registry.getTableReader();
//  Object[] results = reader.loadTables(timeStamp);
//  if (results == null) {
//    return false;
//  }
//  fileOffsets = (OffsetTable) results[0];
//  extensionPoints = (HashtableOfStringAndInt) results[1];
//  nextId = ((Integer) results[2]).intValue();
//  fromCache = true;

//  if (!registry.useLazyCacheLoading()) {
//    //TODO Here we could grow all the tables to the right size (ReferenceMap)
//    reader.setHoldObjects(true);
//    markOrphansHasDirty(getOrphans());
//    fromCache = reader.readAllCache(this);
//    formerContributions = getFormerContributions();
//  }
//  return fromCache;
}

void RegistryObjectManager::AddContribution(const SmartPointer<RegistryContribution>& contribution)
{
  QMutexLocker l(&mutex);
  isDirty = true;
  QString Id = contribution->GetKey();

  KeyedElement::Pointer existingContribution = GetFormerContributions().GetByKey(Id);
  if (existingContribution.IsNotNull())
  { // move it from former to new contributions
    RemoveContribution(Id);
    newContributions.Add(existingContribution);
  }
  else
  {
    existingContribution = newContributions.GetByKey(Id);
  }

  if (existingContribution.IsNotNull()) // merge
    existingContribution.Cast<RegistryContribution>()->MergeContribution(contribution);
  else
    newContributions.Add(contribution);

  UpdateNamespaceIndex(contribution, true);
}

QList<int> RegistryObjectManager::GetExtensionPointsFrom(const QString& id) const
{
  QMutexLocker l(&mutex);
  return this->GetExtensionPointsFrom_unlocked(id);
}

QList<int> RegistryObjectManager::GetExtensionPointsFrom_unlocked(const QString& id) const
{
  KeyedElement::Pointer tmp = newContributions.GetByKey(id);
  if (tmp.IsNull())
    tmp = GetFormerContributions().GetByKey(id);
  if (tmp.IsNull())
    return QList<int>();
  return tmp.Cast<RegistryContribution>()->GetExtensionPoints();
}

bool RegistryObjectManager::HasContribution(const QString& id) const
{
  KeyedElement::Pointer result = newContributions.GetByKey(id);
  if (result.IsNull())
    result = GetFormerContributions().GetByKey(id);
  return result.IsNotNull();
}

void RegistryObjectManager::Remove(int id, bool release)
{
  QMutexLocker l(&mutex);
  this->Remove_unlocked(id, release);
}

void RegistryObjectManager::Remove_unlocked(int id, bool release)
{
  RegistryObject::Pointer toRemove = cache->Get(id);
//  if (fileOffsets != null)
//    fileOffsets.removeKey(id);
  if (toRemove.IsNotNull())
    Remove(toRemove, release);
}

SmartPointer<ExtensionPoint> RegistryObjectManager::GetExtensionPointObject(const QString& xptUniqueId) const
{
  QMutexLocker l(&mutex);
  return this->GetExtensionPointObject_unlocked(xptUniqueId);
}

SmartPointer<ExtensionPoint> RegistryObjectManager::GetExtensionPointObject_unlocked(const QString& xptUniqueId) const
{
  HashtableOfStringAndInt::const_iterator idIter = extensionPoints.find(xptUniqueId);
  if (idIter == extensionPoints.end())
    return ExtensionPoint::Pointer();
  return GetObject(idIter.value(), EXTENSION_POINT).Cast<ExtensionPoint>();
}

QList<SmartPointer<ExtensionPointHandle> > RegistryObjectManager::GetExtensionPointsHandles() const
{
  QMutexLocker l(&mutex);
  QList<ExtensionPointHandle::Pointer> result;
  foreach(Handle::Pointer handle, GetHandles(extensionPoints.values(), EXTENSION_POINT))
  {
    result.push_back(handle.Cast<ExtensionPointHandle>());
  }
  return result;
}

SmartPointer<ExtensionPointHandle> RegistryObjectManager::GetExtensionPointHandle(const QString& xptUniqueId)
{
  QMutexLocker l(&mutex);
  HashtableOfStringAndInt::const_iterator idIter = extensionPoints.find(xptUniqueId);
  if (idIter == extensionPoints.end())
    return ExtensionPointHandle::Pointer();
  return GetHandle(idIter.value(), EXTENSION_POINT).Cast<ExtensionPointHandle>();
}

QList<int> RegistryObjectManager::GetExtensionsFrom(const QString& contributorId) const
{
  QMutexLocker l(&mutex);
  return this->GetExtensionsFrom_unlocked(contributorId);
}

QList<int> RegistryObjectManager::GetExtensionsFrom_unlocked(const QString& contributorId) const
{
  KeyedElement::Pointer tmp = newContributions.GetByKey(contributorId);
  if (tmp.IsNull())
    tmp = GetFormerContributions().GetByKey(contributorId);
  if (tmp.IsNull())
    return QList<int>();
  return tmp.Cast<RegistryContribution>()->GetExtensions();
}

bool RegistryObjectManager::AddExtensionPoint(const SmartPointer<ExtensionPoint>& currentExtPoint, bool hold)
{
  QMutexLocker l(&mutex);
  QString uniqueId = currentExtPoint->GetUniqueIdentifier();
  if (extensionPoints.contains(uniqueId))
    return false;
  Add(currentExtPoint, hold);
  extensionPoints.insert(uniqueId, currentExtPoint->GetObjectId());
  return true;
}

void RegistryObjectManager::RemoveExtensionPoint(const QString& extensionPointId)
{
  QMutexLocker l(&mutex);
  HashtableOfStringAndInt::iterator pointIdIter = extensionPoints.find(extensionPointId);
  if (pointIdIter == extensionPoints.end())
    return;

  int pointId = pointIdIter.value();
  extensionPoints.erase(pointIdIter);
  Remove_unlocked(pointId, true);
}

void RegistryObjectManager::RemoveContribution(const QString& contributorId)
{
  QMutexLocker l(&mutex);
  bool removed = newContributions.RemoveByKey(contributorId);
  if (removed == false)
  {
    removed = GetFormerContributions().RemoveByKey(contributorId);
    if (removed)
      formerContributions = GetFormerContributions(); //This forces the removed namespace to stay around, so we do not forget about removed namespaces
  }

  if (removed)
  {
    isDirty = true;
    return;
  }
}

void RegistryObjectManager::AddOrphans(const QString& extensionPoint, const QList<int>& extensions)
{
  OrphansMapType& orphans = GetOrphans();
  orphans[extensionPoint].append(extensions);

}

void RegistryObjectManager::AddOrphan(const QString& extensionPoint, int extension)
{
  OrphansMapType& orphans = GetOrphans();
  orphans[extensionPoint].push_back(extension);
}

QList<int> RegistryObjectManager::RemoveOrphans(const QString& extensionPoint)
{
  OrphansMapType& orphans = GetOrphans();
  return orphans.take(extensionPoint);
}

void RegistryObjectManager::RemoveOrphan(const QString& extensionPoint, int extension)
{
  OrphansMapType& orphans = GetOrphans();
  OrphansMapType::iterator existingOrphanExtensionsIter = orphans.find(extensionPoint);

  if (existingOrphanExtensionsIter == orphans.end())
    return;

  existingOrphanExtensionsIter.value().removeAll(extension);
  if (existingOrphanExtensionsIter.value().empty())
  {
    orphans.erase(existingOrphanExtensionsIter);
  }
}

QHash<QString, QList<int> > RegistryObjectManager::GetOrphanExtensions() const
{
  return GetOrphans();
}

int RegistryObjectManager::GetNextId() const
{
  return nextId;
}

HashtableOfStringAndInt RegistryObjectManager::GetExtensionPoints() const
  {
  return extensionPoints;
}

QList<KeyedHashSet> RegistryObjectManager::GetContributions() const
{
  QList<KeyedHashSet> result;
  result.push_back(newContributions);
  result.push_back(GetFormerContributions());
  return result;
}

RegistryObjectManager::ContributorsMapType& RegistryObjectManager::GetContributors() const
{
  if (!contributorsLoaded)
  {
    if (fromCache)
    {
      //contributors = registry->GetTableReader()->LoadContributors();
    }
    contributorsLoaded = true;
  }
  return contributors;
}

SmartPointer<RegistryContributor> RegistryObjectManager::GetContributor(const QString& id) const
{
  QMutexLocker l(&mutex);
  RegistryContributor::Pointer contributor = GetContributors().value(id);
  if (contributor.IsNotNull())
    return contributor;
  // check if we have it among removed contributors - potentially
  // notification of removals might be processed after the contributor
  // marked as removed:
  if (!removedContributors.empty())
  {
    return removedContributors.value(id);
  }
  return RegistryContributor::Pointer();
}

void RegistryObjectManager::AddContributor(const SmartPointer<RegistryContributor>& newContributor)
{
  QMutexLocker l(&mutex);
  QString key = newContributor->GetActualId();
  if (!GetContributors().contains(key))
  {
    isDirty = true;
    if (!removedContributors.empty())
    {
      removedContributors.remove(key);
    }
    GetContributors().insert(key, newContributor);
  }
}

void RegistryObjectManager::RemoveContributor(const QString& id)
{
  QMutexLocker l(&mutex);
  isDirty = true;
  RegistryContributor::Pointer removed = GetContributors().take(id);
  if (removed.IsNotNull())
  {
    removedContributors.insert(id, removed);
  }
}

KeyedHashSet& RegistryObjectManager::GetNamespacesIndex() const
{
  if (!namespacesIndexLoaded)
  {
    if (fromCache)
    {
      //namespacesIndex = registry->GetTableReader()->LoadNamespaces();
    }
    namespacesIndexLoaded = true;
  }
  return namespacesIndex;
}

QHash<int, SmartPointer<RegistryObject> > RegistryObjectManager::GetAssociatedObjects(const QString& contributionId) const
{
  QMutexLocker l(&mutex);
  //Collect all the objects associated with this contribution
  QList<int> xpts = GetExtensionPointsFrom_unlocked(contributionId);
  QList<int> exts = GetExtensionsFrom_unlocked(contributionId);
  QHash<int, RegistryObject::Pointer> actualObjects;
  for (int i = 0; i < exts.size(); i++)
  {
    RegistryObject::Pointer tmp = BasicGetObject(exts[i], RegistryObjectManager::EXTENSION);
    actualObjects.insert(exts[i], tmp);
    CollectChildren(tmp, 0, actualObjects);
  }
  for (int i = 0; i < xpts.size(); i++)
  {
    RegistryObject::Pointer xpt = BasicGetObject(xpts[i], RegistryObjectManager::EXTENSION_POINT);
    actualObjects.insert(xpts[i], xpt);
  }

  return actualObjects;
}

void RegistryObjectManager::AddAssociatedObjects(QHash<int, SmartPointer<RegistryObject> >& map,
                            const SmartPointer<RegistryObject>& registryObject)
{
  QMutexLocker l(&mutex);
  CollectChildren(registryObject, 0, map);
}

void RegistryObjectManager::AddNavigableObjects(QHash<int, SmartPointer<RegistryObject> >& associatedObjects)
{
  QMutexLocker l(&mutex);
  typedef QHash<int, RegistryObject::Pointer> ObjectsMapType;
  ObjectsMapType result;
  for (ObjectsMapType::ConstIterator iter = associatedObjects.begin(); iter != associatedObjects.end(); ++iter)
  {
    const RegistryObject::Pointer& object = iter.value();
    if (Extension::Pointer extension = object.Cast<Extension>())
    {
      // add extension point
      ExtensionPoint::Pointer extPoint = GetExtensionPointObject(extension->GetExtensionPointIdentifier());
      if (extPoint.IsNull()) // already removed?
        continue;

      if (!associatedObjects.contains(extPoint->GetObjectId()))
        result.insert(extPoint->GetObjectId(), extPoint);

      // add all extensions for the extension point
      QList<int> extensions = extPoint->GetRawChildren();
      for (int j = 0; j < extensions.size(); j++)
      {
        RegistryObject::Pointer tmp = BasicGetObject(extensions[j], RegistryObjectManager::EXTENSION);
        if (tmp.IsNull()) // already removed
          continue;
        if (!associatedObjects.contains(extensions[j]))
        {
          result.insert(extensions[j], tmp);
          CollectChildren(tmp, 0, result);
        }
      }
    }
    else if (ExtensionPoint::Pointer extPoint = object.Cast<ExtensionPoint>())
    {
      // by now extensions of this extension point have been marked as orphans
      OrphansMapType& orphans = GetOrphans();
      QString name = extPoint->GetUniqueIdentifier();
      QList<int> extensions = orphans.value(name);
      for (int j = 0; j < extensions.size(); j++)
      {
        RegistryObject::Pointer tmp = BasicGetObject(extensions[j], RegistryObjectManager::EXTENSION);
        if (tmp.IsNull()) // already removed
          continue;
        if (!associatedObjects.contains(extensions[j]))
        {
          result.insert(extensions[j], tmp);
          CollectChildren(tmp, 0, result);
        }
      }
    }
  }

  for (ObjectsMapType::ConstIterator iter = result.begin(); iter != result.end(); ++iter)
  {
    associatedObjects.insert(iter.key(), iter.value());
  }
}

void RegistryObjectManager::RemoveObjects(const QHash<int, SmartPointer<RegistryObject> >& associatedObjects)
{
  QMutexLocker l(&mutex);
  //Remove the objects from the main object manager so they can no longer be accessed.
  foreach (RegistryObject::Pointer toRemove, associatedObjects)
  {
    Remove(toRemove->GetObjectId(), true);
    if (ExtensionPoint::Pointer extPoint = toRemove.Cast<ExtensionPoint>())
    {
      RemoveExtensionPoint(extPoint->GetUniqueIdentifier());
    }
  }
}

SmartPointer<IObjectManager> RegistryObjectManager::CreateDelegatingObjectManager(const QHash<int, SmartPointer<RegistryObject> > &object)
{
  IObjectManager::Pointer objMgr(new TemporaryObjectManager(object, this));
  return objMgr;
}

bool RegistryObjectManager::UnlinkChildFromContributions(int id)
{
  QMutexLocker l(&mutex);
  if (UnlinkChildFromContributions(newContributions.Elements(), id))
    return true;
  return UnlinkChildFromContributions(GetFormerContributions().Elements(), id);
}

QString RegistryObjectManager::FindCommonNamespaceIdentifier(const QList<SmartPointer<RegistryObject> >& registryObjects) const
{
  QString namespaceName;
  for (int i = 0; i < registryObjects.size(); i++)
  {
    RegistryObject::Pointer currentObject = registryObjects[i];
    QString tmp;
    if (ExtensionPoint::Pointer extPoint = currentObject.Cast<ExtensionPoint>())
    {
      tmp = extPoint->GetNamespace();
    }
    else if (Extension::Pointer extension = currentObject.Cast<Extension>())
    {
      tmp = extension->GetNamespaceIdentifier();
    }

    if (namespaceName.isNull())
    {
      namespaceName = tmp;
      continue;
    }
    if (namespaceName != tmp)
    {
      return QString();
    }
  }
  return namespaceName;
}

void RegistryObjectManager::RemoveExtensionPointFromNamespaceIndex(int extensionPoint, const QString& namespaceName)
{
  QMutexLocker l(&mutex);
  RegistryIndexElement::Pointer indexElement = GetNamespaceIndex(namespaceName);
  indexElement->UpdateExtensionPoint(extensionPoint, false);
}

void RegistryObjectManager::RemoveExtensionFromNamespaceIndex(int extensions, const QString& namespaceName)
{
  QMutexLocker l(&mutex);
  RegistryIndexElement::Pointer indexElement = GetNamespaceIndex(namespaceName);
  indexElement->UpdateExtension(extensions, false);
}

void RegistryObjectManager::UpdateNamespaceIndex(const SmartPointer<RegistryContribution>& contribution, bool added)
{
  // if all extension points are from the same namespace combine them in one block and add them all together
  QList<int> contribExtensionPoints = contribution->GetExtensionPoints();
  QList<RegistryObject::Pointer> extensionPointObjects = GetObjects_unlocked(contribExtensionPoints, EXTENSION_POINT);
  QString commonExptsNamespace;
  if (contribExtensionPoints.size() > 1)
  {
    commonExptsNamespace = FindCommonNamespaceIdentifier(extensionPointObjects);
  }
  if (!commonExptsNamespace.isEmpty())
  {
    RegistryIndexElement::Pointer indexElement = GetNamespaceIndex(commonExptsNamespace);
    indexElement->UpdateExtensionPoints(contribExtensionPoints, added);
  }
  else
  {
    for (int i = 0; i < contribExtensionPoints.size(); i++)
    {
      QString namespaceName = extensionPointObjects[i].Cast<ExtensionPoint>()->GetNamespace();
      RegistryIndexElement::Pointer indexElement = GetNamespaceIndex(namespaceName);
      indexElement->UpdateExtensionPoint(contribExtensionPoints[i], added);
    }
  }

  // if all extensions are from the same namespace combine them in one block and add them all together
  QList<int> contrExtensions = contribution->GetExtensions();
  QList<RegistryObject::Pointer> extensionObjects = GetObjects_unlocked(contrExtensions, EXTENSION);
  QString commonExtNamespace;
  if (contrExtensions.size() > 1)
  {
    commonExtNamespace = FindCommonNamespaceIdentifier(extensionObjects);
  }
  if (!commonExtNamespace.isEmpty())
  {
    RegistryIndexElement::Pointer indexElement = GetNamespaceIndex(commonExtNamespace);
    indexElement->UpdateExtensions(contrExtensions, added);
  }
  else
  {
    for (int i = 0; i < contrExtensions.size(); i++)
    {
      QString namespaceName = extensionObjects[i].Cast<Extension>()->GetNamespaceIdentifier();
      RegistryIndexElement::Pointer indexElement = GetNamespaceIndex(namespaceName);
      indexElement->UpdateExtension(contrExtensions[i], added);
    }
  }
}

KeyedHashSet& RegistryObjectManager::GetFormerContributions() const
{
  if (!formerContributionsLoaded)
  {
    if (fromCache)
    {
      //formerContributions = registry->GetTableReader()->LoadContributions();
    }
    formerContributionsLoaded = true;
  }
  return formerContributions;
}

void RegistryObjectManager::Remove(const SmartPointer<RegistryObject>& registryObject, bool release)
{
  cache->Remove(registryObject->GetObjectId());
  if (release)
    Release(registryObject);
}

void RegistryObjectManager::Hold(const SmartPointer<RegistryObject>& toHold)
{
  heldObjects.Add(toHold);
}

void RegistryObjectManager::Release(const SmartPointer<RegistryObject>& toRelease)
{
  heldObjects.Remove(toRelease);
}

SmartPointer<RegistryObject> RegistryObjectManager::BasicGetObject(int id, short type) const
{
  RegistryObject::Pointer result = cache->Get(id);
  if (result.IsNotNull())
    return result;
  if (fromCache)
    result = Load(id, type);
  if (result.IsNull())
    throw InvalidRegistryObjectException();
  cache->Put(id, result);
  return result;
}

SmartPointer<RegistryObject> RegistryObjectManager::Load(int /*id*/, short /*type*/) const
{
//  TableReader reader = registry.getTableReader();
//  if (fileOffsets == null)
//    return null;
//  int offset = fileOffsets.get(id);
//  if (offset == Integer.MIN_VALUE)
//    return null;
//  switch (type) {
//    case CONFIGURATION_ELEMENT :
//      return reader.loadConfigurationElement(offset);

//    case THIRDLEVEL_CONFIGURATION_ELEMENT :
//      return reader.loadThirdLevelConfigurationElements(offset, this);

//    case EXTENSION :
//      return reader.loadExtension(offset);

//    case EXTENSION_POINT :
//    default : //avoid compile errors. type must always be known
//      return reader.loadExtensionPointTree(offset, this);
//  }
  return RegistryObject::Pointer();
}

RegistryObjectManager::OrphansMapType& RegistryObjectManager::GetOrphans() const
{
  if (!orphanExtensionsLoaded)
  {
    if(fromCache)
    {
      //orphanExtensions = registry->GetTableReader()->LoadOrphans();
    }
    orphanExtensionsLoaded = true;
  }
  return orphanExtensions;
}

SmartPointer<RegistryIndexElement> RegistryObjectManager::GetNamespaceIndex(const QString& namespaceName) const
{
  RegistryIndexElement::Pointer indexElement = GetNamespacesIndex().GetByKey(namespaceName).Cast<RegistryIndexElement>();
  if (indexElement.IsNull())
  {
    indexElement = new RegistryIndexElement(namespaceName);
    namespacesIndex.Add(indexElement);
  }
  return indexElement;
}

void RegistryObjectManager::CollectChildren(const SmartPointer<RegistryObject>& ce, int level,
                                            QHash<int, SmartPointer<RegistryObject> >& collector) const
{
  QList<RegistryObject::Pointer> children =
      GetObjects(ce->GetRawChildren(),
                 level == 0 || ce->NoExtraData() ? RegistryObjectManager::CONFIGURATION_ELEMENT : RegistryObjectManager::THIRDLEVEL_CONFIGURATION_ELEMENT);
  for (int j = 0; j < children.size(); j++)
  {
    collector.insert(children[j]->GetObjectId(), children[j]);
    CollectChildren(children[j], level + 1, collector);
  }
}

bool RegistryObjectManager::UnlinkChildFromContributions(const QList<SmartPointer<KeyedElement> >& contributions, int id)
{
  for (int i = 0; i < contributions.size(); i++)
  {
    RegistryContribution::Pointer candidate = contributions[i].Cast<RegistryContribution>();
    if (candidate.IsNull())
      continue;
    if (candidate->HasChild(id))
    {
      candidate->UnlinkChild(id);
      if (candidate->IsEmpty())
        RemoveContribution(candidate->GetContributorId());
      return true;
    }
  }
  return false;
}

}
