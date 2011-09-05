/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#include "mitkServiceListenerEntry_p.h"

namespace mitk {

class ServiceListenerEntryData : public SharedData
{
public:

  ServiceListenerEntryData(Module* mc, const ServiceListenerDelegate& l, const std::string& filter)
    : mc(mc), listener(l.Clone()), bRemoved(false), ldap()
  {
    if (!filter.empty())
    {
      ldap = LDAPExpr(filter);
    }
  }

  ~ServiceListenerEntryData()
  {
    delete listener;
  }

  Module* const mc;
  const ServiceListenerDelegate* const listener;
  bool bRemoved;
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
   * where <code>attr</code> is one of {@link Constants#OBJECTCLASS},
   * {@link Constants#SERVICE_ID} or {@link Constants#SERVICE_PID}, and
   * <code>value</code> must not contain a wildcard character.
   * <p>
   * The index of the vector determines which key the cache is for
   * (see {@link ServiceListenerState#hashedKeys}). For each key, there is
   * a vector pointing out the values which are accepted by this
   * ServiceListenerEntry's filter. This cache is maintained to make
   * it easy to remove this service listener.
   */
  LDAPExpr::LocalCache local_cache;
};

ServiceListenerEntry::ServiceListenerEntry(const ServiceListenerEntry& other)
  : d(other.d)
{

}

ServiceListenerEntry::~ServiceListenerEntry()
{

}

ServiceListenerEntry& ServiceListenerEntry::operator=(const ServiceListenerEntry& other)
{
  d = other.d;
  return *this;
}

bool ServiceListenerEntry::operator==(const ServiceListenerEntry& other) const
{
  return ((d->mc == 0 || other.d->mc == 0) || d->mc == other.d->mc) &&
      (d->listener->operator ==(other.d->listener));
}

bool ServiceListenerEntry::operator<(const ServiceListenerEntry& other) const
{
  return d->mc < other.d->mc;
}

void ServiceListenerEntry::SetRemoved(bool removed) const
{
  d->bRemoved = removed;
}

bool ServiceListenerEntry::IsRemoved() const
{
  return d->bRemoved;
}

ServiceListenerEntry::ServiceListenerEntry(Module* mc, const ServiceListenerDelegate& l, const std::string& filter)
  : d(new ServiceListenerEntryData(mc, l, filter))
{

}

Module* ServiceListenerEntry::GetModule() const
{
  return d->mc;
}

std::string ServiceListenerEntry::GetFilter() const
{
  return d->ldap.IsNull() ? std::string() : d->ldap.ToString();
}

LDAPExpr ServiceListenerEntry::GetLDAPExpr() const
{
  return d->ldap;
}

LDAPExpr::LocalCache& ServiceListenerEntry::GetLocalCache() const
{
  return d->local_cache;
}

void ServiceListenerEntry::CallDelegate(const ServiceEvent& event) const
{
  d->listener->Execute(event);
}

}
