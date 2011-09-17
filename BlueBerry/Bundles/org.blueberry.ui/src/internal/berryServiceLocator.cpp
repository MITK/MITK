/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "berryServiceLocator.h"

#include "berryWorkbenchServiceRegistry.h"

#include "../services/berryIServiceFactory.h"
#include "../services/berryINestable.h"

#include <Poco/Exception.h>

namespace berry
{

ServiceLocator::ParentLocator::ParentLocator(const IServiceLocator::WeakPtr parent,
    const std::string& serviceInterface) :
  locator(parent), key(serviceInterface)
{

}

Object::Pointer ServiceLocator::ParentLocator::GetService(
    const std::string& api)
{
  if (key == api)
  {
    try {
      return locator.Lock()->GetService(key);
    }
    catch (const BadWeakPointerException& /*e*/)
    {

    }
  }
  return Object::Pointer(0);
}

bool ServiceLocator::ParentLocator::HasService(const std::string& api) const
{
  if (key == api)
  {
    return true;
  }
  return false;
}

ServiceLocator::ServiceLocator() :
  activated(false), disposed(false)
{

}

ServiceLocator::ServiceLocator(const IServiceLocator::WeakPtr _parent,
    const IServiceFactory::ConstPointer _factory, IDisposable::WeakPtr _owner) :
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
    Object::Pointer service = serviceItr->second;
    if (INestable::Pointer nestableService = service.Cast<INestable>())
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
    Object::Pointer service = serviceItr->second;
    if (INestable::Pointer nestableService = service.Cast<INestable>())
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
    Object::Pointer object = serviceItr->second;
    if (IDisposable::Pointer service = object.Cast<IDisposable>())
    {
      service->Dispose();
    }
  }
  services.clear();

  parent.Reset();
  disposed = true;
}

Object::Pointer ServiceLocator::GetService(const std::string& key)
{
  if (disposed)
  {
    return Object::Pointer(0);
  }

  KeyToServiceMapType::const_iterator iter = services.find(key);
  Object::Pointer service;

  if (iter != services.end())
  {
    service = iter->second;
  }
  else
  {
    // if we don't have a service in our cache then:
    // 1. check our local factory
    // 2. go to the registry
    // or 3. use the parent service
    IServiceLocator::Pointer factoryParent(WorkbenchServiceRegistry::GLOBAL_PARENT);
    if (!parent.Expired())
    {
      factoryParent = new ParentLocator(parent, key);
    }
    if (factory)
    {
      service = factory->Create(key, factoryParent, IServiceLocator::Pointer(this));
    }
    if (!service)
    {
      service = WorkbenchServiceRegistry::GetRegistry()->GetService(key,
          factoryParent, ServiceLocator::Pointer(this));
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

bool ServiceLocator::HasService(const std::string& key) const
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

void ServiceLocator::RegisterService(const std::string& api,
    Object::Pointer service) const
{
  if (api.empty())
  {
    throw Poco::InvalidArgumentException("The service key cannot be empty"); //$NON-NLS-1$
  }

//  if (!api.isInstance(service))
//  {
//    throw new IllegalArgumentException("The service does not implement the given interface"); //$NON-NLS-1$
//  }

  if (services.find(api) != services.end())
  {
    Object::Pointer currentService = services[api];
    services.erase(api);
    if (IDisposable::Pointer disposable = currentService.Cast<IDisposable>())
    {
      disposable->Dispose();
    }
  }

  if (service)
  {
    services.insert(std::make_pair(api, service));
    if (INestable::Pointer nestable = service.Cast<INestable>())
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

void ServiceLocator::UnregisterServices(const std::vector<std::string>& /*serviceNames*/)
{
  IDisposable::Pointer d(owner);
  if (d)
  {
    d->Dispose();
  }
}

}
