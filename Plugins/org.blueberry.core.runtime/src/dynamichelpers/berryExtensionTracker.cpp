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

#include "berryExtensionTracker.h"

#include <berryIExtension.h>
#include <berryIExtensionPoint.h>
#include <berryIExtensionPointFilter.h>
#include <berryIExtensionRegistry.h>
#include <berryIExtensionChangeHandler.h>
#include <berryIRegistryEventListener.h>
#include <berryListenerList.h>
#include <berryLog.h>
#include "internal/berrySimpleExtensionPointFilter.h"
#include <berryPlatform.h>

#include <QSet>
#include <QHash>
#include <QMutex>

namespace berry {

struct ExtensionTracker::Impl
{
  struct HandlerWrapper;

  ExtensionTracker* const q;
  QHash<IExtension::Pointer, QSet<Object::Pointer> > extensionToStrongObjects;
  QHash<IExtension::Pointer, QSet<Object::WeakPtr> > extensionToWeakObjects;
  //ListenerList<HandlerWrapper> handlers;
  QHash<IExtensionChangeHandler*, HandlerWrapper*> handlerToWrapper;
  QMutex mutex;
  bool closed;
  IExtensionRegistry* registry; // the registry that this tacker works with

  Impl(ExtensionTracker* q, IExtensionRegistry* theRegistry)
    : q(q)
    , closed(false)
    , registry(theRegistry)
  {}

};

struct ExtensionTracker::Impl::HandlerWrapper : public IRegistryEventListener
{
  ExtensionTracker* const q;
  IExtensionChangeHandler* const handler;

  HandlerWrapper(ExtensionTracker* q, IExtensionChangeHandler* handler)
    : q(q)
    , handler(handler)
  {}

  bool operator==(const HandlerWrapper& target) const
  {
    return handler == target.handler;
  }

  void Added(const QList<IExtension::Pointer>& extensions) override
  {
    for (int i = 0; i < extensions.size(); ++i)
    {
      q->ApplyAdd(handler, extensions[i]);
    }
  }

  void Removed(const QList<IExtension::Pointer>& extensions) override
  {
    QList<QList<Object::Pointer> > removedObjects;
    {
      QMutexLocker lock(&q->d->mutex);
      for (int i = 0; i < extensions.size(); ++i)
      {
        QSet<Object::Pointer> associatedObjects = q->d->extensionToStrongObjects.take(extensions[i]);
        foreach(const Object::WeakPtr& ptr, q->d->extensionToWeakObjects.take(extensions[i]))
        {
          if (!ptr.Expired())
          {
            associatedObjects.insert(ptr.Lock());
          }
        }
        removedObjects.push_back(associatedObjects.toList());
      }
    }
    for (int i = 0; i < extensions.size(); ++i)
    {
      q->ApplyRemove(handler, extensions[i], removedObjects[i]);
    }
  }

  void Added(const QList<IExtensionPoint::Pointer>& /*extensionPoints*/) override
  {
    // do nothing
  }

  void Removed(const QList<IExtensionPoint::Pointer>& /*extensionPoints*/) override
  {
    // do nothing
  }
};

ExtensionTracker::ExtensionTracker()
{
  this->Init(Platform::GetExtensionRegistry());
}

ExtensionTracker::~ExtensionTracker()
{
}

ExtensionTracker::ExtensionTracker(IExtensionRegistry* theRegistry)
  : d()
{
  this->Init(theRegistry);
}

void ExtensionTracker::Init(IExtensionRegistry* registry)
{
  d.reset(new Impl(this, registry));
  if (registry == nullptr)
  {
    //RuntimeLog.log(new Status(IStatus.ERROR, RegistryMessages.OWNER_NAME, 0, RegistryMessages.registry_no_default, null));
    BERRY_ERROR << "Extension tracker was unable to obtain BlueBerry extension registry.";
    d->closed = true;
  }
}

void ExtensionTracker::RegisterHandler(IExtensionChangeHandler* handler, const IExtensionPointFilter& filter)
{
  QMutexLocker lock(&d->mutex);
  if (d->closed) return;
  auto iter = d->handlerToWrapper.insert(handler, new Impl::HandlerWrapper(this, handler));
  d->registry->AddListener(iter.value(), filter);
}

void ExtensionTracker::RegisterHandler(IExtensionChangeHandler* handler, const QString& extensionPointId)
{
  this->RegisterHandler(handler, extensionPointId.isEmpty() ? IExtensionPointFilter(nullptr)
                                                            : IExtensionPointFilter(new SimpleExtensionPointFilter(extensionPointId)));
}

void ExtensionTracker::UnregisterHandler(IExtensionChangeHandler* handler)
{
  QMutexLocker lock(&d->mutex);
  if (d->closed) return;
  IRegistryEventListener* listener = d->handlerToWrapper.take(handler);
  d->registry->RemoveListener(listener);
  delete listener;
}

void ExtensionTracker::RegisterObject(const SmartPointer<IExtension>& element, const SmartPointer<Object>& object, IExtensionTracker::ReferenceType referenceType)
{
  if (element.IsNull() || object.IsNull()) return;

  QMutexLocker lock(&d->mutex);
  if (d->closed) return;

  if (referenceType == REF_STRONG)
  {
    d->extensionToStrongObjects[element].insert(object);
  }
  else if (referenceType == REF_WEAK)
  {
    d->extensionToWeakObjects[element].insert(Object::WeakPtr(object));
  }
}

QList<SmartPointer<Object> > ExtensionTracker::GetObjects(const IExtension::Pointer& element) const
{
  QSet<Object::Pointer> objectSet;

  QMutexLocker lock(&d->mutex);
  if (d->closed) return objectSet.toList();

  auto iter = d->extensionToStrongObjects.find(element);
  if (iter != d->extensionToStrongObjects.end())
  {
    objectSet.unite(iter.value());
  }
  auto iter2 = d->extensionToWeakObjects.find(element);
  if (iter2 != d->extensionToWeakObjects.end())
  {
    foreach(const Object::WeakPtr& ptr, iter2.value())
    {
      if (!ptr.Expired()) objectSet.insert(ptr.Lock());
    }
  }
  return objectSet.toList();
}

void ExtensionTracker::Close()
{
  QMutexLocker lock(&d->mutex);
  if (d->closed) return;
  foreach(Impl::HandlerWrapper* wrapper, d->handlerToWrapper.values())
  {
    d->registry->RemoveListener(wrapper);
    delete wrapper;
  }
  d->extensionToStrongObjects.clear();
  d->extensionToWeakObjects.clear();
  d->handlerToWrapper.clear();

  d->closed = true;
}

void ExtensionTracker::UnregisterObject(const SmartPointer<IExtension>& extension, const SmartPointer<Object>& object)
{
  QMutexLocker lock(&d->mutex);
  if (d->closed) return;

  auto iter = d->extensionToStrongObjects.find(extension);
  if (iter != d->extensionToStrongObjects.end())
  {
    iter.value().remove(object);
  }
  auto iter2 = d->extensionToWeakObjects.find(extension);
  if (iter2 != d->extensionToWeakObjects.end())
  {
    iter2.value().remove(Object::WeakPtr(object));
  }
}

QList<SmartPointer<Object> > ExtensionTracker::UnregisterObject(const SmartPointer<IExtension>& extension)
{
  QSet<Object::Pointer> objectSet;

  QMutexLocker lock(&d->mutex);
  if (d->closed) return objectSet.toList();

  auto iter = d->extensionToStrongObjects.find(extension);
  if (iter != d->extensionToStrongObjects.end())
  {
    objectSet.unite(iter.value());
    d->extensionToStrongObjects.erase(iter);
  }
  auto iter2 = d->extensionToWeakObjects.find(extension);
  if (iter2 != d->extensionToWeakObjects.end())
  {
    foreach(const Object::WeakPtr& ptr, iter2.value())
    {
      if (!ptr.Expired()) objectSet.insert(ptr.Lock());
    }
    d->extensionToWeakObjects.erase(iter2);
  }

  return objectSet.toList();
}

IExtensionPointFilter ExtensionTracker::CreateExtensionPointFilter(const SmartPointer<IExtensionPoint>& xpt)
{
  struct F : IExtensionPointFilter::Concept {
    const IExtensionPoint::Pointer m_Xpt;

    F(const IExtensionPoint::Pointer& xp)
      : m_Xpt(xp)
    {}

    bool Matches(const IExtensionPoint* target) const override
    {
      return m_Xpt == target;
    }
  };

  return IExtensionPointFilter(new F(xpt));
}

IExtensionPointFilter ExtensionTracker::CreateExtensionPointFilter(const QList<SmartPointer<IExtensionPoint> >& xpts)
{
  struct F : IExtensionPointFilter::Concept {
    const QList<IExtensionPoint::Pointer> m_Xpts;

    F(const QList<IExtensionPoint::Pointer>& xps)
      : m_Xpts(xps)
    {}

    bool Matches(const IExtensionPoint* target) const override
    {
      for (int i = 0; i < m_Xpts.size(); i++)
      {
        if (m_Xpts[i] == target)
        {
          return true;
        }
      }
      return false;
    }
  };

  return IExtensionPointFilter(new F(xpts));
}

IExtensionPointFilter ExtensionTracker::CreateNamespaceFilter(const QString& id)
{
  struct F : IExtensionPointFilter::Concept {
    const QString m_Id;

    F(const QString& id)
      : m_Id(id)
    {}

    bool Matches(const IExtensionPoint* target) const override
    {
      return m_Id == target->GetNamespaceIdentifier();
    }
  };

  return IExtensionPointFilter(new F(id));
}

void ExtensionTracker::ApplyAdd(IExtensionChangeHandler* handler, const SmartPointer<IExtension>& extension)
{
  handler->AddExtension(this, extension);
}

void ExtensionTracker::ApplyRemove(IExtensionChangeHandler* handler, const SmartPointer<IExtension>& removedExtension, const QList<Object::Pointer>& removedObjects)
{
  handler->RemoveExtension(removedExtension, removedObjects);
}

}
