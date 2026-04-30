/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "usServiceListenerEntry_p.h"
#include "usServiceListenerHook_p.h"

#include <cassert>

namespace us {

class ServiceListenerEntryData : public ServiceListenerHook::ListenerInfoData
{
public:

  ServiceListenerEntryData(ModuleContext* mc, const ServiceListenerEntry::ServiceListener& l,
                           void* data, const std::string& filter)
    : ServiceListenerHook::ListenerInfoData(mc, l, data, filter)
    , ldap()
    , hashValue(0)
  {
    if (!filter.empty())
    {
      ldap = LDAPExpr(filter);
    }
  }

  ~ServiceListenerEntryData() override
  {
  }

  LDAPExpr ldap;

  /**
   * The elements of "simple" filters are cached, for easy lookup.
   *
   * The grammar for simple filters is as follows:
   *
   * <pre>
   * Simple = '(' attr '=' value ')'
   *        | '(' '|' Simple+ ')'
   * </pre>
   * where <code>attr</code> is one of Constants#OBJECTCLASS,
   * Constants#SERVICE_ID or Constants#SERVICE_PID, and
   * <code>value</code> must not contain a wildcard character.
   * <p>
   * The index of the vector determines which key the cache is for
   * (see ServiceListenerState#hashedKeys). For each key, there is
   * a vector pointing out the values which are accepted by this
   * ServiceListenerEntry's filter. This cache is maintained to make
   * it easy to remove this service listener.
   */
  LDAPExpr::LocalCache local_cache;

  std::size_t hashValue;

private:

  // purposely not implemented
  ServiceListenerEntryData(const ServiceListenerEntryData&);
  ServiceListenerEntryData& operator=(const ServiceListenerEntryData&);
};

ServiceListenerEntry::ServiceListenerEntry(const ServiceListenerEntry& other)
  : ServiceListenerHook::ListenerInfo(other)
{
}

ServiceListenerEntry::ServiceListenerEntry(const ServiceListenerHook::ListenerInfo& info)
  : ServiceListenerHook::ListenerInfo(info)
{
  assert(info.d);
}

ServiceListenerEntry::~ServiceListenerEntry()
{
}

ServiceListenerEntry& ServiceListenerEntry::operator=(const ServiceListenerEntry& other)
{
  d = other.d;
  return *this;
}

void ServiceListenerEntry::SetRemoved(bool removed) const
{
  d->bRemoved = removed;
}

ServiceListenerEntry::ServiceListenerEntry(ModuleContext* mc, const ServiceListener& l,
                                           void* data, const std::string& filter)
  : ServiceListenerHook::ListenerInfo(new ServiceListenerEntryData(mc, l, data, filter))
{
}

const LDAPExpr& ServiceListenerEntry::GetLDAPExpr() const
{
  return static_cast<ServiceListenerEntryData*>(d.Data())->ldap;
}

LDAPExpr::LocalCache& ServiceListenerEntry::GetLocalCache() const
{
  return static_cast<ServiceListenerEntryData*>(d.Data())->local_cache;
}

void ServiceListenerEntry::CallDelegate(const ServiceEvent& event) const
{
  d->listener(event);
}

bool ServiceListenerEntry::operator==(const ServiceListenerEntry& other) const
{
  return ((d->mc == nullptr || other.d->mc == nullptr) || d->mc == other.d->mc) &&
      (d->data == other.d->data) && ServiceListenerCompare()(d->listener, other.d->listener);
}

std::size_t ServiceListenerEntry::Hash() const
{
  using namespace std;

  if (static_cast<ServiceListenerEntryData*>(d.Data())->hashValue == 0)
  {
    static_cast<ServiceListenerEntryData*>(d.Data())->hashValue =
        ((std::hash<ModuleContext*>()(d->mc) ^ (std::hash<void*>()(d->data) << 1)) >> 1) ^
        (std::hash<US_SERVICE_LISTENER_FUNCTOR>()(d->listener) << 1);
  }
  return static_cast<ServiceListenerEntryData*>(d.Data())->hashValue;
}

}
