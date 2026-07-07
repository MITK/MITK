/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usServiceReferenceBase.h>
#include "usServiceReferenceBasePrivate.h"
#include "usServiceRegistrationBasePrivate.h"

#include <usModule.h>
#include "usModulePrivate.h"

#include <cassert>

namespace us {

ServiceReferenceBase::ServiceReferenceBase()
  : d(new ServiceReferenceBasePrivate(nullptr))
{

}

ServiceReferenceBase::ServiceReferenceBase(const ServiceReferenceBase& ref)
  : d(ref.d)
{
  d->ref.Ref();
}

ServiceReferenceBase::ServiceReferenceBase(ServiceRegistrationBasePrivate* reg)
  : d(new ServiceReferenceBasePrivate(reg))
{
}

void ServiceReferenceBase::SetInterfaceId(const std::string& interfaceId)
{
  if (d->ref > 1)
  {
    // detach
    d->ref.Deref();
    d = new ServiceReferenceBasePrivate(d->registration);
  }
  d->interfaceId = interfaceId;
}

ServiceReferenceBase::operator bool_type() const
{
  return GetModule() != nullptr ? &ServiceReferenceBase::d : nullptr;
}

ServiceReferenceBase& ServiceReferenceBase::operator=(int null)
{
  if (null == 0)
  {
    if (!d->ref.Deref())
      delete d;
    d = new ServiceReferenceBasePrivate(nullptr);
  }
  return *this;
}

ServiceReferenceBase::~ServiceReferenceBase()
{
  if (!d->ref.Deref())
    delete d;
}

Any ServiceReferenceBase::GetProperty(const std::string& key) const
{
  if (d == nullptr || d->registration == nullptr) return Any();

  MutexLock lock(d->registration->propsLock);

  return d->registration->properties.Value(key);
}

void ServiceReferenceBase::GetPropertyKeys(std::vector<std::string>& keys) const
{
  if (d == nullptr || d->registration == nullptr) return;

  MutexLock lock(d->registration->propsLock);

  const std::vector<std::string>& ks = d->registration->properties.Keys();
  keys.assign(ks.begin(), ks.end());
}

Module* ServiceReferenceBase::GetModule() const
{
  if (d->registration == nullptr || d->registration->module == nullptr)
  {
    return nullptr;
  }

  return d->registration->module->q;
}

void ServiceReferenceBase::GetUsingModules(std::vector<Module*>& modules) const
{
  if (d == nullptr || d->registration == nullptr) return;

  MutexLock lock(d->registration->propsLock);

  ServiceRegistrationBasePrivate::ModuleToRefsMap::const_iterator end = d->registration->dependents.end();
  for (ServiceRegistrationBasePrivate::ModuleToRefsMap::const_iterator iter = d->registration->dependents.begin();
       iter != end; ++iter)
  {
    modules.push_back(iter->first);
  }
}

bool ServiceReferenceBase::operator<(const ServiceReferenceBase& reference) const
{
  if (!(*this))
  {
    return true;
  }

  if (!reference)
  {
    return false;
  }

  const Any anyR1 = GetProperty(ServiceConstants::SERVICE_RANKING());
  const Any anyR2 = reference.GetProperty(ServiceConstants::SERVICE_RANKING());
  assert(anyR1.Empty() || anyR1.Type() == typeid(int));
  assert(anyR2.Empty() || anyR2.Type() == typeid(int));
  const int r1 = anyR1.Empty() ? 0 : *any_cast<int>(&anyR1);
  const int r2 = anyR2.Empty() ? 0 : *any_cast<int>(&anyR2);

  if (r1 != r2)
  {
    // use ranking if ranking differs
    return r1 < r2;
  }
  else
  {
    const Any anyId1 = GetProperty(ServiceConstants::SERVICE_ID());
    const Any anyId2 = reference.GetProperty(ServiceConstants::SERVICE_ID());
    assert(anyId1.Type() == typeid(long int));
    assert(anyId2.Type() == typeid(long int));
    const long int id1 = *any_cast<long int>(&anyId1);
    const long int id2 = *any_cast<long int>(&anyId2);

    // otherwise compare using IDs,
    // is less than if it has a higher ID.
    return id2 < id1;
  }
}

bool ServiceReferenceBase::operator==(const ServiceReferenceBase& reference) const
{
  return d->registration == reference.d->registration;
}

bool ServiceReferenceBase::operator==(std::nullptr_t) const noexcept
{
  return d->registration == nullptr;
}

ServiceReferenceBase& ServiceReferenceBase::operator=(const ServiceReferenceBase& reference)
{
  ServiceReferenceBasePrivate* curr_d = d;
  d = reference.d;
  d->ref.Ref();

  if (!curr_d->ref.Deref())
    delete curr_d;

  return *this;
}

bool ServiceReferenceBase::IsConvertibleTo(const std::string& interfaceId) const
{
  return d->IsConvertibleTo(interfaceId);
}

std::string ServiceReferenceBase::GetInterfaceId() const
{
  return d->interfaceId;
}

std::size_t ServiceReferenceBase::Hash() const
{
  using namespace std;
  return std::hash<ServiceRegistrationBasePrivate*>()(this->d->registration);
}

}

using namespace us;

std::ostream& operator<<(std::ostream& os, const ServiceReferenceBase& serviceRef)
{
  if (serviceRef)
  {
    assert(serviceRef.GetModule() != nullptr);

    os << "Reference for service object registered from "
       << serviceRef.GetModule()->GetName() << " " << serviceRef.GetModule()->GetVersion()
       << " (";
    std::vector<std::string> keys;
    serviceRef.GetPropertyKeys(keys);
    size_t keySize = keys.size();
    for(size_t i = 0; i < keySize; ++i)
    {
      os << keys[i] << "=" << serviceRef.GetProperty(keys[i]).ToString();
      if (i < keySize-1) os << ",";
    }
    os << ")";
  }
  else
  {
    os << "Invalid service reference";
  }

  return os;
}
