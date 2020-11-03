/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryServiceLocator.h"

#include "berryWorkbenchServiceRegistry.h"

#include "services/berryIServiceFactory.h"
#include "services/berryINestable.h"


namespace berry
{

ServiceLocator::ParentLocator::ParentLocator(
    IServiceLocator* parent,
    const QString& serviceInterface)
  : locator(parent), key(serviceInterface)
{
}

Object* ServiceLocator::ParentLocator::GetService(const QString& api)
{
  if (key == api)
  {
    try {
      return locator->GetService(key);
    }
    catch (const BadWeakPointerException& /*e*/)
    {

    }
  }
  return nullptr;
}

bool ServiceLocator::ParentLocator::HasService(const QString& api) const
{
  if (key == api)
  {
    return true;
  }
  return false;
}

ServiceLocator::ServiceLocator()
  : activated(false)
  , factory(nullptr)
  , parent(nullptr)
  , disposed(false)
{

}

ServiceLocator::ServiceLocator(IServiceLocator* _parent,
    const IServiceFactory* _factory, IDisposable::WeakPtr _owner) :
  activated(false), factory(_factory), parent(_parent),
  disposed(false), owner(_owner)
{

}

void ServiceLocator::Activate()
{
  activated = true;
  for (KeyToServiceMapType::iterator serviceItr = services.begin(); serviceItr
      != services.end(); ++serviceItr)
  {
    Object* service = serviceItr.value();
    if (INestable* nestableService = dynamic_cast<INestable*>(service))
    {
      nestableService->Activate();
    }
  }

}

void ServiceLocator::Deactivate()
{
  activated = false;
  for (KeyToServiceMapType::iterator serviceItr = services.begin(); serviceItr
      != services.end(); ++serviceItr)
  {
    Object* service = serviceItr.value();
    if (INestable* nestableService = dynamic_cast<INestable*>(service))
    {
      nestableService->Deactivate();
    }
  }

}

void ServiceLocator::Dispose()
{
  for (KeyToServiceMapType::iterator serviceItr = services.begin(); serviceItr
      != services.end(); ++serviceItr)
  {
    Object* object = serviceItr.value();
    if (IDisposable* service = dynamic_cast<IDisposable*>(object))
    {
      service->Dispose();
    }
  }
  services.clear();

  parent = nullptr;
  disposed = true;
}

Object* ServiceLocator::GetService(const QString& key)
{
  if (disposed)
  {
    return nullptr;
  }

  KeyToServiceMapType::const_iterator iter = services.find(key);
  Object* service = nullptr;

  if (iter != services.end())
  {
    service = iter.value();
  }
  else
  {
    // if we don't have a service in our cache then:
    // 1. check our local factory
    // 2. go to the registry
    // or 3. use the parent service
    IServiceLocator::Pointer factoryParent = WorkbenchServiceRegistry::GLOBAL_PARENT;
    if (parent)
    {
      factoryParent = new ParentLocator(parent, key);
    }
    if (factory)
    {
      service = factory->Create(key, factoryParent.GetPointer(), this);
    }
    if (!service)
    {
      Object::Pointer factoryService =
          WorkbenchServiceRegistry::GetRegistry()->GetService(key,
                                                              factoryParent.GetPointer(),
                                                              this);
      if (factoryService)
      {
        managedFactoryServices.push_back(factoryService);
      }
      service = factoryService.GetPointer();
    }
    if (!service)
    {
      service = factoryParent->GetService(key);
    }
    else
    {
      this->RegisterService(key, service);
    }
  }
  return service;
}

bool ServiceLocator::HasService(const QString& key) const
{
  if (disposed)
  {
    return false;
  }
  if (services.find(key) != services.end())
  {
    return true;
  }
  return false;
}

void ServiceLocator::RegisterService(const QString& api,
    Object* service) const
{
  if (api.isEmpty())
  {
    throw ctkInvalidArgumentException("The service key cannot be empty");
  }

//  if (!api.isInstance(service))
//  {
//    throw new IllegalArgumentException("The service does not implement the given interface"); //$NON-NLS-1$
//  }

  if (services.find(api) != services.end())
  {
    Object* currentService = services[api];
    services.remove(api);
    if (IDisposable* disposable = dynamic_cast<IDisposable*>(currentService))
    {
      disposable->Dispose();
    }
  }

  if (service)
  {
    services.insert(api, service);
    if (INestable* nestable = dynamic_cast<INestable*>(service))
    {
      if (activated)
      {
        nestable->Activate();
      }
    }
  }
}

bool ServiceLocator::IsDisposed() const
{
  return disposed;
}

void ServiceLocator::UnregisterServices(const QList<QString>& /*serviceNames*/)
{
  IDisposable::Pointer d(owner);
  if (d)
  {
    d->Dispose();
  }
}

}
