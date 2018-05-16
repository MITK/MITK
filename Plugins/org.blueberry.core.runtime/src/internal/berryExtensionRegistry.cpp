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


#include "berryExtensionRegistry.h"

#include "berryCombinedEventDelta.h"
#include "berryConfigurationElement.h"
#include "berryConfigurationElementAttribute.h"
#include "berryConfigurationElementDescription.h"
#include "berryExtension.h"
#include "berryExtensionHandle.h"
#include "berryExtensionPoint.h"
#include "berryExtensionPointHandle.h"
#include "berryExtensionsParser.h"
#include "berryIConfigurationElement.h"
#include "berryIExtension.h"
#include "berryIExtensionPoint.h"
#include "berrySimpleExtensionPointFilter.h"
#include "berryMultiStatus.h"
#include "berryPlatform.h"
#include "berryRegistryConstants.h"
#include "berryRegistryContribution.h"
#include "berryRegistryContributor.h"
#include "berryRegistryMessages.h"
#include "berryRegistryObjectFactory.h"
#include "berryRegistryObjectManager.h"
#include "berryRegistryProperties.h"
#include "berryRegistryStrategy.h"
#include "berryStatus.h"

#include <QThread>
#include <QTime>

namespace berry {

struct ExtensionRegistry::ListenerInfo {
  IExtensionPointFilter filter;
  IRegistryEventListener* listener;

  ListenerInfo(IRegistryEventListener* listener, const IExtensionPointFilter& filter)
    : filter(filter), listener(listener)
  {
  }

  /**
   * Used by ListenerList to ensure uniqueness.
   */
  bool operator==(const ListenerInfo& another) const
  {
    return another.listener == this->listener;
  }

};

void ExtensionRegistry::Add(const SmartPointer<RegistryContribution> &element)
{
  QWriteLocker l(&access);
  eventDelta = CombinedEventDelta::RecordAddition();
  BasicAdd(element, true);
  FireRegistryChangeEvent();
  eventDelta.Reset();
}

QString ExtensionRegistry::AddExtension(int extension)
{
  Extension::Pointer addedExtension = registryObjects->GetObject(extension, RegistryObjectManager::EXTENSION).Cast<Extension>();
  QString extensionPointToAddTo = addedExtension->GetExtensionPointIdentifier();
  ExtensionPoint::Pointer extPoint = registryObjects->GetExtensionPointObject(extensionPointToAddTo);
  //orphan extension
  if (extPoint.IsNull())
  {
    registryObjects->AddOrphan(extensionPointToAddTo, extension);
    return QString();
  }
  // otherwise, link them
  QList<int> newExtensions = extPoint->GetRawChildren();
  newExtensions.push_back(extension);
  Link(extPoint, newExtensions);
  if (!eventDelta.IsNull())
    eventDelta.RememberExtension(extPoint, extension);
  return extPoint->GetNamespace();
  //return RecordChange(extPoint, extension, ExtensionDelta::ADDED);
}

QString ExtensionRegistry::AddExtensionPoint(int extPoint)
{
  ExtensionPoint::Pointer extensionPoint = registryObjects->GetObject(extPoint, RegistryObjectManager::EXTENSION_POINT).Cast<ExtensionPoint>();
  if (!eventDelta.IsNull())
    eventDelta.RememberExtensionPoint(extensionPoint);
  QList<int> orphans = registryObjects->RemoveOrphans(extensionPoint->GetUniqueIdentifier());
  if (orphans.empty())
    return QString();
  Link(extensionPoint, orphans);
  if (!eventDelta.IsNull())
    eventDelta.RememberExtensions(extensionPoint, orphans);
  return extensionPoint->GetNamespace();
  //return RecordChange(extensionPoint, orphans, ExtensionDelta::ADDED);
}

QSet<QString> ExtensionRegistry::AddExtensionsAndExtensionPoints(const SmartPointer<RegistryContribution>& element)
{
  // now add and resolve extensions and extension points
  QSet<QString> affectedNamespaces;
  QList<int> extPoints = element->GetExtensionPoints();
  for (int i = 0; i < extPoints.size(); i++)
  {
    QString namespaze = this->AddExtensionPoint(extPoints[i]);
    if (!namespaze.isEmpty())
      affectedNamespaces.insert(namespaze);
  }
  QList<int> extensions = element->GetExtensions();
  for (int i = 0; i < extensions.size(); i++)
  {
    QString namespaze = this->AddExtension(extensions[i]);
    if (!namespaze.isEmpty())
      affectedNamespaces.insert(namespaze);
  }
  return affectedNamespaces;
}

void ExtensionRegistry::AddListenerInternal(IRegistryEventListener* listener, const IExtensionPointFilter& filter)
{
  listeners.Add(ListenerInfo(listener, filter));
}

void ExtensionRegistry::BasicAdd(const SmartPointer<RegistryContribution>& element, bool link)
{
  registryObjects->AddContribution(element);
  if (!link)
    return;
  AddExtensionsAndExtensionPoints(element);
  SetObjectManagers(registryObjects->CreateDelegatingObjectManager(
                      registryObjects->GetAssociatedObjects(element->GetContributorId())));
}

void ExtensionRegistry::SetObjectManagers(const SmartPointer<IObjectManager>& manager)
{
  if (!eventDelta.IsNull())
    eventDelta.SetObjectManager(manager);
}

void ExtensionRegistry::BasicRemove(const QString& contributorId)
{
  // ignore anonymous namespaces
  RemoveExtensionsAndExtensionPoints(contributorId);
  QHash<int, RegistryObject::Pointer> associatedObjects = registryObjects->GetAssociatedObjects(contributorId);
  registryObjects->RemoveObjects(associatedObjects);
  registryObjects->AddNavigableObjects(associatedObjects); // put the complete set of navigable objects
  SetObjectManagers(registryObjects->CreateDelegatingObjectManager(associatedObjects));

  registryObjects->RemoveContribution(contributorId);
  registryObjects->RemoveContributor(contributorId);
}

void ExtensionRegistry::FireRegistryChangeEvent()
{
  // if there is nothing to say, just bail out
  if (listeners.IsEmpty())
  {
    return;
  }
  // for thread safety, create tmp collections
  QList<ListenerInfo> tmpListeners = listeners.GetListeners();
  // do the notification asynchronously
  //strategy->ScheduleChangeEvent(tmpListeners, tmpDeltas, this);
  this->ScheduleChangeEvent(tmpListeners, eventDelta);
}

//RegistryDelta ExtensionRegistry::GetDelta(const QString& namespaze) const
//{
//  // is there a delta for the plug-in?
//  RegistryDelta existingDelta = deltas.value(namespaze);
//  if (existingDelta != null)
//    return existingDelta;

//  //if not, create one
//  RegistryDelta delta = new RegistryDelta();
//  deltas.put(namespace, delta);
//  return delta;
//}

void ExtensionRegistry::Link(const SmartPointer<ExtensionPoint>& extPoint, const QList<int>& extensions)
{
  extPoint->SetRawChildren(extensions);
  registryObjects->Add(extPoint, true);
}

//QString ExtensionRegistry::RecordChange(const SmartPointer<ExtensionPoint>& extPoint, int extension, int kind)
//{
//  // avoid computing deltas when there are no listeners
//  if (listeners.isEmpty())
//    return QString();
//  ExtensionDelta extensionDelta = new ExtensionDelta();
//  extensionDelta.setExtension(extension);
//  extensionDelta.setExtensionPoint(extPoint.getObjectId());
//  extensionDelta.setKind(kind);
//  getDelta(extPoint.getNamespace()).addExtensionDelta(extensionDelta);
//  return extPoint.getNamespace();
//}

//QString ExtensionRegistry::RecordChange(const SmartPointer<ExtensionPoint>& extPoint, const QList<int>& extensions, int kind)
//{
//  if (listeners.isEmpty())
//    return null;
//    QString namespace = extPoint.getNamespace();
//  if (extensions == null || extensions.length == 0)
//    return namespace;
//  RegistryDelta pluginDelta = getDelta(extPoint.getNamespace());
//  for (int i = 0; i < extensions.length; i++) {
//    ExtensionDelta extensionDelta = new ExtensionDelta();
//    extensionDelta.setExtension(extensions[i]);
//    extensionDelta.setExtensionPoint(extPoint.getObjectId());
//    extensionDelta.setKind(kind);
//    pluginDelta.addExtensionDelta(extensionDelta);
//  }
//  return namespace;
//}

QString ExtensionRegistry::RemoveExtension(int extensionId)
{
  Extension::Pointer extension = registryObjects->GetObject(extensionId, RegistryObjectManager::EXTENSION).Cast<Extension>();
  registryObjects->RemoveExtensionFromNamespaceIndex(extensionId, extension->GetNamespaceIdentifier());
  QString xptName = extension->GetExtensionPointIdentifier();
  ExtensionPoint::Pointer extPoint = registryObjects->GetExtensionPointObject(xptName);
  if (extPoint.IsNull())
  {
    registryObjects->RemoveOrphan(xptName, extensionId);
    return QString();
  }
  // otherwise, unlink the extension from the extension point
  QList<int> existingExtensions = extPoint->GetRawChildren();
  QList<int> newExtensions;
  if (existingExtensions.size() > 1)
  {
    for (int i = 0; i < existingExtensions.size(); ++i)
      if (existingExtensions[i] != extension->GetObjectId())
        newExtensions.push_back(existingExtensions[i]);
  }
  Link(extPoint, newExtensions);
  if (!eventDelta.IsNull())
    eventDelta.RememberExtension(extPoint, extensionId);
  return extPoint->GetNamespace();
  //return recordChange(extPoint, extension.getObjectId(), IExtensionDelta.REMOVED);
}

QString ExtensionRegistry::RemoveExtensionPoint(int extPoint)
{
  ExtensionPoint::Pointer extensionPoint = registryObjects->GetObject(
        extPoint, RegistryObjectManager::EXTENSION_POINT).Cast<ExtensionPoint>();
  registryObjects->RemoveExtensionPointFromNamespaceIndex(extPoint, extensionPoint->GetNamespace());
  QList<int> existingExtensions = extensionPoint->GetRawChildren();
  if (!existingExtensions.empty())
  {
    registryObjects->AddOrphans(extensionPoint->GetUniqueIdentifier(), existingExtensions);
    Link(extensionPoint, QList<int>());
  }
  if (!eventDelta.IsNull())
  {
    eventDelta.RememberExtensionPoint(extensionPoint);
    eventDelta.RememberExtensions(extensionPoint, existingExtensions);
  }
  return extensionPoint->GetNamespace();
  //return recordChange(extensionPoint, existingExtensions, IExtensionDelta.REMOVED);
}

QSet<QString> ExtensionRegistry::RemoveExtensionsAndExtensionPoints(const QString& contributorId)
{
  QSet<QString> affectedNamespaces;
  QList<int> extensions = registryObjects->GetExtensionsFrom(contributorId);
  for (int i = 0; i < extensions.size(); i++)
  {
    QString namespaze = this->RemoveExtension(extensions[i]);
    if (!namespaze.isEmpty())
      affectedNamespaces.insert(namespaze);
  }

  // remove extension points
  QList<int> extPoints = registryObjects->GetExtensionPointsFrom(contributorId);
  for (int i = 0; i < extPoints.size(); i++)
  {
    QString namespaze = this->RemoveExtensionPoint(extPoints[i]);
    if (!namespaze.isEmpty())
      affectedNamespaces.insert(namespaze);
  }
  return affectedNamespaces;
}

struct ExtensionRegistry::QueueElement
{
  QList<ListenerInfo> listenerInfos;
  CombinedEventDelta scheduledDelta;

  QueueElement()
  {
  }

  QueueElement(const QList<ListenerInfo>& infos, const CombinedEventDelta& delta)
    : listenerInfos(infos), scheduledDelta(delta)
  {
  }
};

class ExtensionRegistry::RegistryEventThread : public QThread
{

private:

  QAtomicInt stop;
  ExtensionRegistry* registry;
  Queue& queue;

public:

  RegistryEventThread(ExtensionRegistry* registry, Queue& queue)
    : stop(0), registry(registry), queue(queue)
  {
    this->setObjectName("Extension Registry Event Dispatcher");
  }

  void interrupt()
  {
    stop.fetchAndStoreOrdered(1);
  }

  void run() override
  {
    while (!stop.fetchAndAddOrdered(0))
    {
      QueueElement element;
      {
        Queue::Locker l(&queue);
        while (queue.empty())
          queue.wait();
        element = queue.takeFirst();
      }
      registry->ProcessChangeEvent(element.listenerInfos, element.scheduledDelta);
    }
  }
};

bool ExtensionRegistry::CheckReadWriteAccess(QObject* key, bool persist) const
  {
  if (masterToken == key)
    return true;
  if (userToken == key && !persist)
    return true;
  return false;
}

void ExtensionRegistry::LogError(const QString& owner, const QString& contributionName, const ctkException& e)
{
  QString message = QString("Could not parse XML contribution for \"%1\". Any contributed extensions "
                            "and extension points will be ignored.").arg(QString(owner) + "/" + contributionName);
  IStatus::Pointer status(new Status(IStatus::ERROR_TYPE, RegistryMessages::OWNER_NAME, 0, message, e, BERRY_STATUS_LOC));
  Log(status);
}

void ExtensionRegistry::CreateExtensionData(const QString& contributorId,
                                            const ConfigurationElementDescription& description,
                                            const SmartPointer<RegistryObject>& parent, bool persist)
{
  ConfigurationElement::Pointer currentConfigurationElement = GetElementFactory()->CreateConfigurationElement(persist);
  currentConfigurationElement->SetContributorId(contributorId);
  currentConfigurationElement->SetName(description.GetName());

  QList<ConfigurationElementAttribute> descriptionProperties = description.GetAttributes();

  QList<QString> properties;
  if (!descriptionProperties.empty())
  {
    for (int i = 0; i < descriptionProperties.size(); i++)
    {
      properties.push_back(descriptionProperties[i].GetName());
      properties.push_back(Translate(descriptionProperties[i].GetValue(), nullptr));
    }
  }
  currentConfigurationElement->SetProperties(properties);

  QString value = description.GetValue();
  if (!value.isEmpty())
    currentConfigurationElement->SetValue(value);

  GetObjectManager()->Add(currentConfigurationElement, true);

  // process children
  QList<ConfigurationElementDescription> children = description.GetChildren();
  if (!children.empty())
  {
    for (int i = 0; i < children.size(); i++)
    {
      CreateExtensionData(contributorId, children[i], currentConfigurationElement, persist);
    }
  }

  QList<int> newValues = parent->GetRawChildren();
  newValues.push_back(currentConfigurationElement->GetObjectId());
  parent->SetRawChildren(newValues);
  currentConfigurationElement->SetParentId(parent->GetObjectId());
  currentConfigurationElement->SetParentType(parent.Cast<ConfigurationElement>() ?
                                               RegistryObjectManager::CONFIGURATION_ELEMENT :
                                               RegistryObjectManager::EXTENSION);
}

bool ExtensionRegistry::RemoveObject(const SmartPointer<RegistryObject>& registryObject,
                  bool isExtensionPoint, QObject* token)
{
  if (!CheckReadWriteAccess(token, registryObject->ShouldPersist()))
    throw ctkInvalidArgumentException("Unauthorized access to the ExtensionRegistry.removeExtension() method. Check if proper access token is supplied.");
  int id = registryObject->GetObjectId();

  QWriteLocker l(&access);
  eventDelta = CombinedEventDelta::RecordRemoval();
  if (isExtensionPoint)
  {
    RemoveExtensionPoint(id);
  }
  else
  {
    RemoveExtension(id);
  }
  QHash<int, RegistryObject::Pointer> removed;
  removed.insert(id, registryObject);
  // There is some asymmetry between extension and extension point removal. Removing extension point makes
  // extensions "orphans" but does not remove them. As a result, only extensions needs to be processed.
  if (!isExtensionPoint)
  {
    registryObjects->AddAssociatedObjects(removed, registryObject);
  }
  registryObjects->RemoveObjects(removed);
  registryObjects->AddNavigableObjects(removed);
  IObjectManager::Pointer manager = registryObjects->CreateDelegatingObjectManager(removed);
  //GetDelta(namespaze)->SetObjectManager(manager);
  //eventDelta->SetObjectManager(manager);

  registryObjects->UnlinkChildFromContributions(id);
  FireRegistryChangeEvent();
  eventDelta.Reset();
  return true;
}

void ExtensionRegistry::SetFileManager(const QString& /*cacheBase*/, bool /*isCacheReadOnly*/)
{
//  if (cacheStorageManager != nullptr)
//    cacheStorageManager->Close(); // close existing file manager first

//  if (cacheBase != null) {
//    cacheStorageManager = new StorageManager(cacheBase, isCacheReadOnly ? "none" : null, isCacheReadOnly); //$NON-NLS-1$
//    try {
//      cacheStorageManager.open(!isCacheReadOnly);
//    } catch (IOException e) {
//      // Ignore the exception. The registry will be rebuilt from source.
//    }
//  }
}

void ExtensionRegistry::EnterRead()
{
  access.lockForRead();
}

void ExtensionRegistry::ExitRead()
{
  access.unlock();
}

void ExtensionRegistry::SetElementFactory()
{
  if (isMultiLanguage)
  {
    throw ctkRuntimeException("Multi-language registry not supported yet.");
    //theRegistryObjectFactory = new RegistryObjectFactoryMulti(this);
  }
  else
  {
    theRegistryObjectFactory.reset(new RegistryObjectFactory(this));
  }
}

//TableReader ExtensionRegistry::getTableReader() const
//{
//  return theTableReader;
//}

bool ExtensionRegistry::CheckCache()
{
//  for (int index = 0; index < strategy.getLocationsLength(); index++) {
//    File possibleCacheLocation = strategy.getStorage(index);
//    if (possibleCacheLocation == null)
//      break; // bail out on the first null
//    setFileManager(possibleCacheLocation, strategy.isCacheReadOnly(index));
//    if (cacheStorageManager != null) {
//      // check this new location:
//      File cacheFile = null;
//      try {
//        cacheFile = cacheStorageManager.lookup(TableReader.getTestFileName(), false);
//      } catch (IOException e) {
//        //Ignore the exception. The registry will be rebuilt from the xml files.
//      }
//      if (cacheFile != null && cacheFile.isFile())
//        return true; // found the appropriate location
//    }
//  }
  return false;
}

void ExtensionRegistry::StopChangeEventScheduler()
{
  if (!eventThread.isNull())
  {
    Queue::Locker l(&queue);
    eventThread->interrupt();
    eventThread->wait();
    eventThread.reset();
  }
}

SmartPointer<RegistryObjectManager> ExtensionRegistry::GetObjectManager() const
{
  return registryObjects;
}

void ExtensionRegistry::AddListener(IRegistryEventListener* listener, const QString& extensionPointId)
{
  AddListenerInternal(listener, extensionPointId.isEmpty() ? IExtensionPointFilter(nullptr)
                                                           : IExtensionPointFilter(new SimpleExtensionPointFilter(extensionPointId)));
}

void ExtensionRegistry::AddListener(IRegistryEventListener* listener, const IExtensionPointFilter& filter)
{
  this->AddListenerInternal(listener, filter);
}

QList<SmartPointer<IConfigurationElement> > ExtensionRegistry::GetConfigurationElementsFor(const QString& extensionPointId) const
{
  // this is just a convenience API - no need to do any sync'ing here
  int lastdot = extensionPointId.lastIndexOf('.');
  if (lastdot == -1)
  {
    QList<IConfigurationElement::Pointer>();
  }
  return GetConfigurationElementsFor(extensionPointId.left(lastdot), extensionPointId.mid(lastdot + 1));
}

QList<SmartPointer<IConfigurationElement> > ExtensionRegistry::GetConfigurationElementsFor(const QString& pluginId,
                                                                        const QString& extensionPointSimpleId) const
{
  // this is just a convenience API - no need to do any sync'ing here
  IExtensionPoint::Pointer extPoint = this->GetExtensionPoint(pluginId, extensionPointSimpleId);
  if (extPoint.IsNull())
    return QList<IConfigurationElement::Pointer>();
  return extPoint->GetConfigurationElements();
}

QList<SmartPointer<IConfigurationElement> > ExtensionRegistry::GetConfigurationElementsFor(const QString& pluginId,
                                                                        const QString& extensionPointName,
                                                                        const QString& extensionId) const
{
  // this is just a convenience API - no need to do any sync'ing here
  IExtension::Pointer extension = this->GetExtension(pluginId, extensionPointName, extensionId);
  if (extension.IsNull())
    return QList<IConfigurationElement::Pointer>();
  return extension->GetConfigurationElements();
}

SmartPointer<IExtension> ExtensionRegistry::GetExtension(const QString& extensionId) const
{
  if (extensionId.isEmpty())
    return IExtension::Pointer();
  int lastdot = extensionId.lastIndexOf('.');
  if (lastdot == -1)
    return IExtension::Pointer();
  QString namespaze = extensionId.left(lastdot);

  QList<ExtensionHandle::Pointer> extensions;
  {
    QReadLocker l(&access);
    extensions = registryObjects->GetExtensionsFromNamespace(namespaze);
  }

  for (int i = 0; i < extensions.size(); i++)
  {
    ExtensionHandle::Pointer suspect = extensions[i];
    if (extensionId == suspect->GetUniqueIdentifier())
      return suspect;
  }
  return IExtension::Pointer();
}

SmartPointer<IExtension> ExtensionRegistry::GetExtension(const QString& extensionPointId, const QString& extensionId) const
{
  // this is just a convenience API - no need to do any sync'ing here
  int lastdot = extensionPointId.lastIndexOf('.');
  if (lastdot == -1)
    return IExtension::Pointer();
  return GetExtension(extensionPointId.left(lastdot), extensionPointId.mid(lastdot + 1), extensionId);
}

SmartPointer<IExtension> ExtensionRegistry::GetExtension(const QString& pluginId,
                                      const QString& extensionPointName,
                                      const QString& extensionId) const
{
  // this is just a convenience API - no need to do any sync'ing here
  IExtensionPoint::Pointer extPoint = GetExtensionPoint(pluginId, extensionPointName);
  if (extPoint.IsNotNull())
    return extPoint->GetExtension(extensionId);
  return IExtension::Pointer();
}

SmartPointer<IExtensionPoint> ExtensionRegistry::GetExtensionPoint(const QString& xptUniqueId) const
{
  QReadLocker l(&access);
  return registryObjects->GetExtensionPointHandle(xptUniqueId);
}

SmartPointer<IExtensionPoint> ExtensionRegistry::GetExtensionPoint(const QString& elementName, const QString& xpt) const
{
  QReadLocker l(&access);
  return registryObjects->GetExtensionPointHandle(elementName + '.' + xpt);
}

QList<SmartPointer<IExtensionPoint> > ExtensionRegistry::GetExtensionPoints() const
{
  QList<ExtensionPointHandle::Pointer> handles;
  {
    QReadLocker l(&access);
    handles = registryObjects->GetExtensionPointsHandles();
  }
  QList<IExtensionPoint::Pointer> result;
  foreach(ExtensionPointHandle::Pointer handle, handles)
  {
    result.push_back(handle);
  }
  return result;
}

QList<SmartPointer<IExtensionPoint> > ExtensionRegistry::GetExtensionPoints(const QString& namespaceName) const
{
  QList<ExtensionPointHandle::Pointer> handles;
  {
    QReadLocker l(&access);
    handles = registryObjects->GetExtensionPointsFromNamespace(namespaceName);
  }
  QList<IExtensionPoint::Pointer> result;
  foreach(ExtensionPointHandle::Pointer handle, handles)
  {
    result.push_back(handle);
  }
  return result;
}

QList<SmartPointer<IExtension> > ExtensionRegistry::GetExtensions(const QString& namespaceName) const
{
  QList<ExtensionHandle::Pointer> handles;
  {
    QReadLocker l(&access);
    handles = registryObjects->GetExtensionsFromNamespace(namespaceName);
  }
  QList<IExtension::Pointer> result;
  foreach (ExtensionHandle::Pointer handle, handles)
  {
    result.push_back(handle);
  }
  return result;
}

QList<SmartPointer<IExtension> > ExtensionRegistry::GetExtensions(const SmartPointer<IContributor>& contributor) const
{
  RegistryContributor::Pointer regContributor = contributor.Cast<RegistryContributor>();
  if (regContributor.IsNull())
    throw ctkInvalidArgumentException("Contributor must be a RegistryContributor."); // should never happen

  QString contributorId = regContributor->GetActualId();

  QList<ExtensionHandle::Pointer> handles;
  {
    QReadLocker l(&access);
    handles = registryObjects->GetExtensionsFromContributor(contributorId);
  }
  QList<IExtension::Pointer> result;
  foreach (ExtensionHandle::Pointer handle, handles)
  {
    result.push_back(handle);
  }
  return result;
}

QList<SmartPointer<IExtensionPoint> > ExtensionRegistry::GetExtensionPoints(const SmartPointer<IContributor>& contributor) const
{
  RegistryContributor::Pointer regContributor = contributor.Cast<RegistryContributor>();
  if (regContributor.IsNull())
    throw ctkInvalidArgumentException("Contributor must be a RegistryContributor."); // should never happen

  QString contributorId = regContributor->GetActualId();

  QList<ExtensionPointHandle::Pointer> handles;
  {
    QReadLocker l(&access);
    handles = registryObjects->GetExtensionPointsFromContributor(contributorId);
  }
  QList<IExtensionPoint::Pointer> result;
  foreach (ExtensionPointHandle::Pointer handle, handles)
  {
    result.push_back(handle);
  }
  return result;
}

QList<QString> ExtensionRegistry::GetNamespaces() const
{
  QReadLocker l(&access);
  QList<KeyedElement::Pointer> namespaceElements = registryObjects->GetNamespacesIndex().Elements();
  QList<QString> namespaceNames;
  for (int i = 0; i < namespaceElements.size(); i++)
  {
    namespaceNames.push_back(namespaceElements[i]->GetKey());
  }
  return namespaceNames;
}

bool ExtensionRegistry::HasContributor(const SmartPointer<IContributor>& contributor) const
{
  RegistryContributor::Pointer regContributor = contributor.Cast<RegistryContributor>();
  if (regContributor.IsNull())
    throw ctkInvalidArgumentException("Contributor must be a RegistryContributor."); // should never happen

  QString contributorId = regContributor->GetActualId();
  return HasContributor(contributorId);
}

bool ExtensionRegistry::HasContributor(const QString& contributorId) const
{
  QReadLocker l(&access);
  return registryObjects->HasContribution(contributorId);
}

void ExtensionRegistry::Remove(const QString& removedContributorId, long timestamp)
{
  Remove(removedContributorId);
  if (timestamp != 0)
    aggregatedTimestamp.Remove(timestamp);
}

void ExtensionRegistry::RemoveContributor(const SmartPointer<IContributor>& contributor, QObject* key)
{
  RegistryContributor::Pointer regContributor = contributor.Cast<RegistryContributor>();
  if (regContributor.IsNull())
    throw ctkInvalidArgumentException("Contributor must be a RegistryContributor."); // should never happen

  if (!CheckReadWriteAccess(key, true))
    throw ctkInvalidArgumentException("Unauthorized access to the ExtensionRegistry.removeContributor() method. Check if proper access token is supplied.");
  QString contributorId = regContributor->GetActualId();
  Remove(contributorId);
}

void ExtensionRegistry::Remove(const QString& removedContributorId)
{
  QWriteLocker l(&access);
  eventDelta = CombinedEventDelta::RecordRemoval();
  BasicRemove(removedContributorId);
  FireRegistryChangeEvent();
  eventDelta.Reset();
}

void ExtensionRegistry::RemoveListener(IRegistryEventListener* listener)
{
  listeners.Remove(ListenerInfo(listener, IExtensionPointFilter(nullptr)));
}

ExtensionRegistry::ExtensionRegistry(RegistryStrategy* registryStrategy, QObject* masterToken, QObject* userToken)
  : registryObjects(nullptr), isMultiLanguage(false), mlErrorLogged(false), eventThread(nullptr)
{
  isMultiLanguage = RegistryProperties::GetProperty(RegistryConstants::PROP_REGISTRY_MULTI_LANGUAGE) == "true";

  if (registryStrategy != nullptr)
    strategy.reset(registryStrategy);
  else
    strategy.reset(new RegistryStrategy(QList<QString>(), QList<bool>(), nullptr));

  this->masterToken = masterToken;
  this->userToken = userToken;
  registryObjects = new RegistryObjectManager(this);

  bool isRegistryFilledFromCache = false; // indicates if registry was able to use cache to populate it's content

  if (strategy->CacheUse())
  {
    // Try to read the registry from the cache first. If that fails, create a new registry
    QTime timer;
    if (Debug())
      timer.start();

    //The cache is made of several files, find the real names of these other files. If all files are found, try to initialize the objectManager
    if (CheckCache())
    {
      // TODO Registry Cache
//      try {
//        theTableReader.setTableFile(cacheStorageManager.lookup(TableReader.TABLE, false));
//        theTableReader.setExtraDataFile(cacheStorageManager.lookup(TableReader.EXTRA, false));
//        theTableReader.setMainDataFile(cacheStorageManager.lookup(TableReader.MAIN, false));
//        theTableReader.setContributionsFile(cacheStorageManager.lookup(TableReader.CONTRIBUTIONS, false));
//        theTableReader.setContributorsFile(cacheStorageManager.lookup(TableReader.CONTRIBUTORS, false));
//        theTableReader.setNamespacesFile(cacheStorageManager.lookup(TableReader.NAMESPACES, false));
//        theTableReader.setOrphansFile(cacheStorageManager.lookup(TableReader.ORPHANS, false));
//        long timestamp = strategy.getContributionsTimestamp();
//        isRegistryFilledFromCache = registryObjects.init(timestamp);
//        if (isRegistryFilledFromCache)
//          aggregatedTimestamp.set(timestamp);
//      } catch (IOException e) {
//        // The registry will be rebuilt from the xml files. Make sure to clear anything filled
//        // from cache so that we won't have partially filled items.
//        isRegistryFilledFromCache = false;
//        clearRegistryCache();
//        log(new Status(IStatus.ERROR, RegistryMessages.OWNER_NAME, 0, RegistryMessages.registry_bad_cache, e));
//      }
    }

//    if (!isRegistryFilledFromCache)
//    {
//      // set cache storage manager to a first writable location
//      for (int index = 0; index < strategy.getLocationsLength(); index++) {
//        if (!strategy.isCacheReadOnly(index)) {
//          setFileManager(strategy.getStorage(index), false);
//          break;
//        }
//      }
//    }

    if (Debug() && isRegistryFilledFromCache)
      BERRY_INFO << "Reading registry cache: " << timer.elapsed() << "ms";

    if (Debug())
    {
      if (!isRegistryFilledFromCache)
        BERRY_INFO << "Reloading registry from manifest files...";
      else
        BERRY_INFO << "Using registry cache...";
    }
  }

  if (DebugEvents())
  {
    struct DebugRegistryListener : public IRegistryEventListener
    {
      void Added(const QList<IExtension::Pointer>& extensions) override
      {
        BERRY_INFO << "Registry extensions ADDED:";
        foreach(IExtension::Pointer extension, extensions)
        {
          BERRY_INFO << "\t" << extension->GetExtensionPointUniqueIdentifier() << " - "
                     << extension->GetNamespaceIdentifier() << "." << extension->GetSimpleIdentifier();
        }
      }

      void Removed(const QList<IExtension::Pointer>& extensions) override
      {
        BERRY_INFO << "Registry extensions REMOVED:";
        foreach(IExtension::Pointer extension, extensions)
        {
          BERRY_INFO << "\t" << extension->GetExtensionPointUniqueIdentifier() << " - "
                     << extension->GetNamespaceIdentifier() << "." << extension->GetSimpleIdentifier();
        }
      }

      void Added(const QList<IExtensionPoint::Pointer>& extensionPoints) override
      {
        BERRY_INFO << "Registry extension-points ADDED:";
        foreach(IExtensionPoint::Pointer extensionPoint, extensionPoints)
        {
          BERRY_INFO << "\t" << extensionPoint->GetUniqueIdentifier();
        }
      }

      void Removed(const QList<IExtensionPoint::Pointer>& extensionPoints) override
      {
        BERRY_INFO << "Registry extension-points REMOVED:";
        foreach(IExtensionPoint::Pointer extensionPoint, extensionPoints)
        {
          BERRY_INFO << "\t" << extensionPoint->GetUniqueIdentifier();
        }
      }
    };

    debugRegistryListener.reset(new DebugRegistryListener());
    AddListener(debugRegistryListener.data());
  }

  // Do extra start processing if specified in the registry strategy
  strategy->OnStart(this, isRegistryFilledFromCache);
}

ExtensionRegistry::~ExtensionRegistry()
{
}

void ExtensionRegistry::Stop(QObject* /*key*/)
{
  // If the registry creator specified a key token, check that the key mathches it
  // (it is assumed that registry owner keeps the key to prevent unautorized accesss).
  if (masterToken != nullptr && masterToken != nullptr)
  {
    throw ctkInvalidArgumentException("Unauthorized access to the ExtensionRegistry.stop() method. Check if proper access token is supplied."); //$NON-NLS-1$
  }

  // Do extra stop processing if specified in the registry strategy
  strategy->OnStop(this);

  StopChangeEventScheduler();

//  if (cacheStorageManager == nullptr)
//    return;

//  if (!registryObjects.isDirty() || cacheStorageManager.isReadOnly()) {
//    cacheStorageManager.close();
//    theTableReader.close();
//    return;
//  }

//  File tableFile = null;
//  File mainFile = null;
//  File extraFile = null;
//  File contributionsFile = null;
//  File contributorsFile = null;
//  File namespacesFile = null;
//  File orphansFile = null;

//  TableWriter theTableWriter = new TableWriter(this);

//  try {
//    cacheStorageManager.lookup(TableReader.TABLE, true);
//    cacheStorageManager.lookup(TableReader.MAIN, true);
//    cacheStorageManager.lookup(TableReader.EXTRA, true);
//    cacheStorageManager.lookup(TableReader.CONTRIBUTIONS, true);
//    cacheStorageManager.lookup(TableReader.CONTRIBUTORS, true);
//    cacheStorageManager.lookup(TableReader.NAMESPACES, true);
//    cacheStorageManager.lookup(TableReader.ORPHANS, true);
//    tableFile = File.createTempFile(TableReader.TABLE, ".new", cacheStorageManager.getBase()); //$NON-NLS-1$
//    mainFile = File.createTempFile(TableReader.MAIN, ".new", cacheStorageManager.getBase()); //$NON-NLS-1$
//    extraFile = File.createTempFile(TableReader.EXTRA, ".new", cacheStorageManager.getBase()); //$NON-NLS-1$
//    contributionsFile = File.createTempFile(TableReader.CONTRIBUTIONS, ".new", cacheStorageManager.getBase()); //$NON-NLS-1$
//    contributorsFile = File.createTempFile(TableReader.CONTRIBUTORS, ".new", cacheStorageManager.getBase()); //$NON-NLS-1$
//    namespacesFile = File.createTempFile(TableReader.NAMESPACES, ".new", cacheStorageManager.getBase()); //$NON-NLS-1$
//    orphansFile = File.createTempFile(TableReader.ORPHANS, ".new", cacheStorageManager.getBase()); //$NON-NLS-1$
//    theTableWriter.setTableFile(tableFile);
//    theTableWriter.setExtraDataFile(extraFile);
//    theTableWriter.setMainDataFile(mainFile);
//    theTableWriter.setContributionsFile(contributionsFile);
//    theTableWriter.setContributorsFile(contributorsFile);
//    theTableWriter.setNamespacesFile(namespacesFile);
//    theTableWriter.setOrphansFile(orphansFile);
//  } catch (IOException e) {
//    cacheStorageManager.close();
//    return; //Ignore the exception since we can recompute the cache
//  }
//  try {
//    long timestamp;
//    // A bit of backward compatibility: if registry was modified, but timestamp was not,
//    // it means that the new timestamp tracking mechanism was not used. In this case
//    // explicitly obtain timestamps for all contributions. Note that this logic
//    // maintains a problem described in the bug 104267 for contributions that
//    // don't use the timestamp tracking mechanism.
//    if (aggregatedTimestamp.isModifed())
//      timestamp = aggregatedTimestamp.getContentsTimestamp(); // use timestamp tracking
//    else
//      timestamp = strategy.getContributionsTimestamp(); // use legacy approach

//    if (theTableWriter.saveCache(registryObjects, timestamp))
//      cacheStorageManager.update(new   QString[] {TableReader.TABLE, TableReader.MAIN, TableReader.EXTRA, TableReader.CONTRIBUTIONS, TableReader.CONTRIBUTORS, TableReader.NAMESPACES, TableReader.ORPHANS}, new   QString[] {tableFile.getName(), mainFile.getName(), extraFile.getName(), contributionsFile.getName(), contributorsFile.getName(), namespacesFile.getName(), orphansFile.getName()});
//  } catch (IOException e) {
//    //Ignore the exception since we can recompute the cache
//  }
//  theTableReader.close();
//  cacheStorageManager.close();
}

void ExtensionRegistry::ClearRegistryCache()
{
//  QString[] keys = new   QString[] {TableReader.TABLE, TableReader.MAIN, TableReader.EXTRA, TableReader.CONTRIBUTIONS, TableReader.ORPHANS};
//  for (int i = 0; i < keys.length; i++)
//    try {
//      cacheStorageManager.remove(keys[i]);
//    } catch (IOException e) {
//      log(new Status(IStatus.ERROR, RegistryMessages.OWNER_NAME, IStatus.ERROR, RegistryMessages.meta_registryCacheReadProblems, e));
//    }
  aggregatedTimestamp.Reset();
}

RegistryObjectFactory* ExtensionRegistry::GetElementFactory()
{
  if (theRegistryObjectFactory.isNull())
    SetElementFactory();
  return theRegistryObjectFactory.data();
}

void ExtensionRegistry::Log(const SmartPointer<IStatus>& status) const
{
  strategy->Log(status);
}

QString ExtensionRegistry::Translate(const QString& key, QTranslator* resources) const
{
  if (isMultiLanguage)
    return key;
  return strategy->Translate(key, resources);
}

bool ExtensionRegistry::Debug() const
{
  return strategy->Debug();
}

bool ExtensionRegistry::DebugEvents() const
{
  return strategy->DebugRegistryEvents();
}

bool ExtensionRegistry::UseLazyCacheLoading() const
{
  return strategy->CacheLazyLoading();
}

long ExtensionRegistry::ComputeState() const
{
  return strategy->GetContainerTimestamp();
}

QObject* ExtensionRegistry::CreateExecutableExtension(const SmartPointer<RegistryContributor>& defaultContributor,
                                                      const QString& className, const QString& requestedContributorName)
{
  return strategy->CreateExecutableExtension(defaultContributor, className, requestedContributorName);
}

void ExtensionRegistry::ProcessChangeEvent(
    const QList<ListenerInfo>& listenerInfos, const CombinedEventDelta& scheduledDelta)
{
  for (int i = 0; i < listenerInfos.size(); i++)
  {
    const ListenerInfo& listenerInfo = listenerInfos[i];

    IRegistryEventListener* extensionListener = listenerInfo.listener;
    QList<IExtension::Pointer> extensions = scheduledDelta.GetExtensions(listenerInfo.filter);
    QList<IExtensionPoint::Pointer> extensionPoints = scheduledDelta.GetExtensionPoints(listenerInfo.filter);

    // notification order - on addition: extension points; then extensions
    if (scheduledDelta.IsAddition())
    {
      if (!extensionPoints.empty())
        extensionListener->Added(extensionPoints);
      if (!extensions.empty())
        extensionListener->Added(extensions);
    }
    else
    { // on removal: extensions; then extension points
      if (!extensions.empty())
        extensionListener->Removed(extensions);
      if (!extensionPoints.empty())
        extensionListener->Removed(extensionPoints);
    }
  }
  IObjectManager::Pointer manager = scheduledDelta.GetObjectManager();
  if (manager.IsNotNull())
    manager->Close();
}

void ExtensionRegistry::ScheduleChangeEvent(const QList<ListenerInfo>& listenerInfos,
                                            const CombinedEventDelta& scheduledDelta)
{
  QueueElement newElement(listenerInfos, scheduledDelta);
  if (eventThread.isNull())
  {
    eventThread.reset(new RegistryEventThread(this, queue));
    eventThread->start();
  }

  {
    Queue::Locker l(&queue);
    queue.push_back(newElement);
    queue.notify();
  }
}

bool ExtensionRegistry::AddContribution(QIODevice* is, const SmartPointer<IContributor>& contributor,
                                        bool persist, const QString& contributionName,
                                        QTranslator* translationBundle, QObject* key, long timestamp)
{
  bool result = AddContribution(is, contributor, persist, contributionName, translationBundle, key);
  if (timestamp != 0)
    aggregatedTimestamp.Add(timestamp);
  return result;
}

bool ExtensionRegistry::AddContribution(QIODevice* is, const SmartPointer<IContributor>& contributor, bool persist,
                     const QString& contributionName, QTranslator* translationBundle, QObject* key)
{
  if (!CheckReadWriteAccess(key, persist))
    throw ctkInvalidArgumentException("Unauthorized access to the ExtensionRegistry::AddContribution() method. Check if proper access token is supplied.");

  RegistryContributor::Pointer internalContributor = contributor.Cast<RegistryContributor>();
  registryObjects->AddContributor(internalContributor); // only adds a contributor if it is not already present

  QString ownerName = internalContributor->GetActualName();
  QString message = QString("Problems parsing plug-in manifest for: \"%1\".").arg(ownerName);
  MultiStatus::Pointer problems(new MultiStatus(RegistryMessages::OWNER_NAME,
                                                ExtensionsParser::PARSE_PROBLEM, message, BERRY_STATUS_LOC));
  ExtensionsParser parser(problems, this);
  RegistryContribution::Pointer contribution =
      GetElementFactory()->CreateContribution(internalContributor->GetActualId(), persist);

  try
  {
    QXmlInputSource xmlInput(is);
    bool success = parser.parseManifest(strategy->GetXMLParser(), &xmlInput, contributionName,
                                        GetObjectManager().GetPointer(), contribution, translationBundle);
    int status = problems->GetSeverity();
    if (status != IStatus::OK_TYPE || !success)
    {
      Log(problems);
      if (status == IStatus::ERROR_TYPE || status == IStatus::CANCEL_TYPE || !success)
        return false;
    }
  }
  catch (const ctkException& e)
  {
    LogError(ownerName, contributionName, e);
    return false;
  }

  Add(contribution); // the add() method does synchronization
  return true;
}

bool ExtensionRegistry::AddExtensionPoint(const QString& identifier, const SmartPointer<IContributor>& contributor,
                       bool persist, const QString& label, const QString& schemaReference, QObject* token)
{
  if (!CheckReadWriteAccess(token, persist))
    throw ctkInvalidArgumentException("Unauthorized access to the ExtensionRegistry::AddExtensionPoint() method. Check if proper access token is supplied.");

  RegistryContributor::Pointer internalContributor = contributor.Cast<RegistryContributor>();
  registryObjects->AddContributor(internalContributor); // only adds a contributor if it is not already present
  QString contributorId = internalContributor->GetActualId();

  // Extension point Id might not be null
  if (identifier.isEmpty())
  {
    QString message = QString("Missing ID for the extension point \"%1\". Element ignored.").arg(label);
    IStatus::Pointer status(new Status(IStatus::ERROR_TYPE, RegistryMessages::OWNER_NAME, 0, message, BERRY_STATUS_LOC));
    Log(status);
  }

  // addition wraps in a contribution
  RegistryContribution::Pointer contribution = GetElementFactory()->CreateContribution(contributorId, persist);
  ExtensionPoint::Pointer currentExtPoint = GetElementFactory()->CreateExtensionPoint(persist);

  QString uniqueId;
  QString namespaceName;
  int simpleIdStart = identifier.lastIndexOf('.');
  if (simpleIdStart == -1)
  {
    namespaceName = contribution->GetDefaultNamespace();
    uniqueId = namespaceName + '.' + identifier;
  }
  else
  {
    namespaceName = identifier.left(simpleIdStart);
    uniqueId = identifier;
  }
  currentExtPoint->SetUniqueIdentifier(uniqueId);
  currentExtPoint->SetNamespace(namespaceName);
  QString labelNLS = Translate(label, nullptr);
  currentExtPoint->SetLabel(labelNLS);
  currentExtPoint->SetSchema(schemaReference);

  if (!GetObjectManager()->AddExtensionPoint(currentExtPoint, true))
  {
    if (Debug())
    {
      QString msg = QString("Ignored duplicate extension point \"%1\" supplied by \"%2\".").arg(uniqueId).arg(contribution->GetDefaultNamespace());
      IStatus::Pointer status(new Status(IStatus::ERROR_TYPE, RegistryMessages::OWNER_NAME, 0, msg, BERRY_STATUS_LOC));
      Log(status);
    }
    return false;
  }

  currentExtPoint->SetContributorId(contributorId);

  // array format: {Number of extension points, Number of extensions, Extension Id}
  QList<int> contributionChildren;
  // Put the extension points into this namespace
  contributionChildren.push_back(1);
  contributionChildren.push_back(0);
  contributionChildren.push_back(currentExtPoint->GetObjectId());

  contribution->SetRawChildren(contributionChildren);

  Add(contribution);
  return true;
}

bool ExtensionRegistry::AddExtension(const QString& identifier, const SmartPointer<IContributor>& contributor,
                  bool persist, const QString& label, const QString& extensionPointId,
                  const ConfigurationElementDescription& configurationElements, QObject* token)
{
  if (!CheckReadWriteAccess(token, persist))
    throw ctkInvalidArgumentException("Unauthorized access to the ExtensionRegistry::AddExtensionPoint() method. Check if proper access token is supplied.");

  // prepare namespace information
  RegistryContributor::Pointer internalContributor = contributor.Cast<RegistryContributor>();
  registryObjects->AddContributor(internalContributor); // only adds a contributor if it is not already present
  QString contributorId = internalContributor->GetActualId();

  // addition wraps in a contribution
  RegistryContribution::Pointer contribution = GetElementFactory()->CreateContribution(contributorId, persist);
  Extension::Pointer currentExtension = GetElementFactory()->CreateExtension(persist);

  QString simpleId;
  QString namespaceName;
  int simpleIdStart = identifier.lastIndexOf('.');
  if (simpleIdStart != -1)
  {
    simpleId = identifier.mid(simpleIdStart + 1);
    namespaceName = identifier.left(simpleIdStart);
  }
  else
  {
    simpleId = identifier;
    namespaceName = contribution->GetDefaultNamespace();
  }
  currentExtension->SetSimpleIdentifier(simpleId);
  currentExtension->SetNamespaceIdentifier(namespaceName);

  QString extensionLabelNLS = Translate(label, nullptr);
  currentExtension->SetLabel(extensionLabelNLS);

    QString targetExtensionPointId;
  if (extensionPointId.indexOf('.') == -1) // No dots -> namespace name added at the start
    targetExtensionPointId = contribution->GetDefaultNamespace() + '.' + extensionPointId;
  else
    targetExtensionPointId = extensionPointId;
  currentExtension->SetExtensionPointIdentifier(targetExtensionPointId);

  // if we have an Id specified, check for duplicates. Only issue warning if duplicate found
  // as it might still work fine - depending on the access pattern.
  if (!simpleId.isNull() && Debug())
  {
    QString uniqueId = namespaceName + '.' + simpleId;
    IExtension::Pointer existingExtension = GetExtension(uniqueId);
    if (existingExtension.IsNotNull())
    {
      QString currentSupplier = contribution->GetDefaultNamespace();
      QString existingSupplier = existingExtension->GetContributor()->GetName();
      QString msg = QString("Extensions supplied by \"%1\" and \"%2\" have the same Id: \"%3\".")
          .arg(currentSupplier).arg(existingSupplier).arg(uniqueId);
      IStatus::Pointer status(new Status(IStatus::WARNING_TYPE, RegistryMessages::OWNER_NAME, 0, msg, BERRY_STATUS_LOC));
      Log(status);
      return false;
    }
  }

  GetObjectManager()->Add(currentExtension, true);

  CreateExtensionData(contributorId, configurationElements, currentExtension, persist);

  currentExtension->SetContributorId(contributorId);

  QList<int> contributionChildren;

  contributionChildren.push_back(0);
  contributionChildren.push_back(1);
  contributionChildren.push_back(currentExtension->GetObjectId());
  contribution->SetRawChildren(contributionChildren);

  Add(contribution);
  return true;
}

bool ExtensionRegistry::RemoveExtension(const SmartPointer<IExtension>& extension, QObject* token)
{
  ExtensionHandle::Pointer handle = extension.Cast<ExtensionHandle>();
  if (handle.IsNull())
    return false;
  return RemoveObject(handle->GetObject(), false, token);
}

bool ExtensionRegistry::RemoveExtensionPoint(const SmartPointer<IExtensionPoint>& extensionPoint, QObject* token)
{
  ExtensionPointHandle::Pointer handle = extensionPoint.Cast<ExtensionPointHandle>();
  if (handle.IsNull())
    return false;
  return RemoveObject(handle->GetObject(), true, token);
}

QList<SmartPointer<IContributor> > ExtensionRegistry::GetAllContributors() const
{
  QList<IContributor::Pointer> result;

  QReadLocker l(&access);
  foreach(RegistryContributor::Pointer contributor, registryObjects->GetContributors().values())
  {
    result.push_back(contributor);
  }
  return result;
}

bool ExtensionRegistry::IsMultiLanguage() const
{
  return isMultiLanguage;
}

QList<QString> ExtensionRegistry::Translate(const QList<QString>& nonTranslated, const SmartPointer<IContributor>& contributor,
                         const QLocale& locale) const
{
  return strategy->Translate(nonTranslated, contributor, locale);
}

QLocale ExtensionRegistry::GetLocale() const
{
  return strategy->GetLocale();
}

void ExtensionRegistry::LogMultiLangError() const
{
  if (mlErrorLogged) // only log this error ones
    return;

  IStatus::Pointer status(new Status(IStatus::ERROR_TYPE, RegistryMessages::OWNER_NAME, 0,
                                     QString("The requested multi-language operation is not enabled. See runtime option \"")
                                     + RegistryConstants::PROP_REGISTRY_MULTI_LANGUAGE + "\".", ctkInvalidArgumentException(""), BERRY_STATUS_LOC));
  Log(status);
  mlErrorLogged = true;
}

}
