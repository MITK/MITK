/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usServiceObjects.h>

#include "usServiceReferenceBasePrivate.h"

#include <set>

namespace us {

class ServiceObjectsBasePrivate
{
public:

  AtomicInt ref;

  ModuleContext* m_context;
  ServiceReferenceBase m_reference;

  // This is used by all ServiceObjects<S> instances with S != void
  std::map<void*, InterfaceMap> m_serviceInstances;
  // This is used by ServiceObjects<void>
  std::set<InterfaceMap> m_serviceInterfaceMaps;

  ServiceObjectsBasePrivate(ModuleContext* context, const ServiceReferenceBase& reference)
    : m_context(context)
    , m_reference(reference)
  {}

  InterfaceMap GetServiceInterfaceMap()
  {
    InterfaceMap result;

    bool isPrototypeScope = m_reference.GetProperty(ServiceConstants::SERVICE_SCOPE()).ToString() ==
                            ServiceConstants::SCOPE_PROTOTYPE();

    if (isPrototypeScope)
    {
      result = m_reference.d->GetPrototypeService(m_context->GetModule());
    }
    else
    {
      result = m_reference.d->GetServiceInterfaceMap(m_context->GetModule());
    }

    return result;
  }
};

ServiceObjectsBase::ServiceObjectsBase(ModuleContext* context, const ServiceReferenceBase& reference)
  : d(new ServiceObjectsBasePrivate(context, reference))
{
  if (!reference)
  {
    delete d;
    throw std::invalid_argument("The service reference is invalid");
  }
  d->ref.Ref();
}

void* ServiceObjectsBase::GetService() const
{
  if (!d->m_reference)
  {
    return nullptr;
  }

  InterfaceMap im = d->GetServiceInterfaceMap();
  void* result = im.find(d->m_reference.GetInterfaceId())->second;

  if (result)
  {
    d->m_serviceInstances.insert(std::make_pair(result, im));
  }
  return result;
}

InterfaceMap ServiceObjectsBase::GetServiceInterfaceMap() const
{
  InterfaceMap result;
  if (!d->m_reference)
  {
    return result;
  }

  result = d->GetServiceInterfaceMap();

  if (!result.empty())
  {
    d->m_serviceInterfaceMaps.insert(result);
  }
  return result;
}

void ServiceObjectsBase::UngetService(void* service)
{
  if (service == nullptr)
  {
    return;
  }

  std::map<void*,InterfaceMap>::iterator serviceIter = d->m_serviceInstances.find(service);
  if (serviceIter == d->m_serviceInstances.end())
  {
    throw std::invalid_argument("The provided service has not been retrieved via this ServiceObjects instance");
  }

  if (!d->m_reference.d->UngetPrototypeService(d->m_context->GetModule(), serviceIter->second))
  {
    MITK_WARN << "Ungetting service unsuccessful";
  }
  else
  {
    d->m_serviceInstances.erase(serviceIter);
  }
}

void ServiceObjectsBase::UngetService(const InterfaceMap& interfaceMap)
{
  if (interfaceMap.empty())
  {
    return;
  }

  std::set<InterfaceMap>::iterator serviceIter = d->m_serviceInterfaceMaps.find(interfaceMap);
  if (serviceIter == d->m_serviceInterfaceMaps.end())
  {
    throw std::invalid_argument("The provided service has not been retrieved via this ServiceObjects instance");
  }

  if (!d->m_reference.d->UngetPrototypeService(d->m_context->GetModule(), interfaceMap))
  {
    MITK_WARN << "Ungetting service unsuccessful";
  }
  else
  {
    d->m_serviceInterfaceMaps.erase(serviceIter);
  }
}

ServiceReferenceBase ServiceObjectsBase::GetReference() const
{
  return d->m_reference;
}

ServiceObjectsBase::ServiceObjectsBase(const ServiceObjectsBase& other)
  : d(other.d)
{
  d->ref.Ref();
}

ServiceObjectsBase::~ServiceObjectsBase()
{
  if (!d->ref.Deref())
  {
    delete d;
  }
}

ServiceObjectsBase& ServiceObjectsBase::operator =(const ServiceObjectsBase& other)
{
  ServiceObjectsBasePrivate* curr_d = d;
  d = other.d;
  d->ref.Ref();

  if (!curr_d->ref.Deref())
    delete curr_d;

  return *this;
}

InterfaceMap ServiceObjects<void>::GetService() const
{
  return this->ServiceObjectsBase::GetServiceInterfaceMap();
}

void ServiceObjects<void>::UngetService(const InterfaceMap& service)
{
  this->ServiceObjectsBase::UngetService(service);
}

ServiceReferenceU ServiceObjects<void>::GetServiceReference() const
{
  return this->ServiceObjectsBase::GetReference();
}

ServiceObjects<void>::ServiceObjects(ModuleContext* context, const ServiceReferenceU& reference)
  : ServiceObjectsBase(context, reference)
{}

}
