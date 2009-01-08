/*=========================================================================

 Program:   openCherry Platform
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

#include "cherryServiceLocator.h"

#include "cherryWorkbenchServiceRegistry.h"

#include <Poco/Exception.h>

namespace cherry
{

ServiceLocator::ParentLocator::ParentLocator(const IServiceLocator* parent,
    const std::string& serviceInterface) :
  locator(parent), key(serviceInterface)
{

}

Object::Pointer ServiceLocator::ParentLocator::GetService(
    const std::string& api) const
{
  if (key == api)
  {
    return locator->GetService(key);
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
  activated(false), factory(0), parent(0), owner(0)
{

}

ServiceLocator::ServiceLocator(IServiceLocator* _parent,
    IServiceFactory* _factory, IDisposable* _owner) :
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
    if (dynamic_cast<INestable*> (service.GetPointer()) != 0)
    {
      INestable* nestableService =
          dynamic_cast<INestable*> (service.GetPointer());
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
    if (dynamic_cast<INestable*> (service.GetPointer()) != 0)
    {
      INestable* nestableService =
          dynamic_cast<INestable*> (service.GetPointer());
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
    if (dynamic_cast<IDisposable*> (object.GetPointer()) != 0)
    {
      IDisposable* service = dynamic_cast<IDisposable*> (object.GetPointer());
      service->Dispose();
    }
  }
  services.clear();

  parent = 0;
  disposed = true;
}

Object::Pointer ServiceLocator::GetService(const std::string& key) const
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
    const IServiceLocator* factoryParent = WorkbenchServiceRegistry::GLOBAL_PARENT;
    if (parent != 0)
    {
      factoryParent = new ParentLocator(parent, key);
    }
    if (factory != 0)
    {
      service = factory->Create(key, factoryParent, this);
    }
    if (service == 0)
    {
      service = WorkbenchServiceRegistry::GetRegistry()->GetService(key,
          factoryParent, this);
    }
    if (service == 0)
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
  if (api == "")
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
    if (dynamic_cast<IDisposable*> (currentService.GetPointer()) != 0)
    {
      IDisposable* disposable =
          dynamic_cast<IDisposable*> (currentService.GetPointer());
      disposable->Dispose();
    }
  }

  if (service != 0)
  {
    services.insert(std::make_pair(api, service));
    if (dynamic_cast<INestable*> (service.GetPointer()) != 0 && activated)
    {
      dynamic_cast<INestable*> (service.GetPointer())->Activate();
    }
  }
}

bool ServiceLocator::IsDisposed()
{
  return disposed;
}

void ServiceLocator::UnregisterServices(const std::vector<std::string>& serviceNames)
{
  if (owner != 0)
  {
    owner->Dispose();
  }
}

}
