/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef USSERVICELISTENERENTRY_H
#define USSERVICELISTENERENTRY_H

#include <usUtils_p.h>
#include <usServiceListenerHook.h>
#include <usListenerFunctors_p.h>

#include "usLDAPExpr_p.h"

namespace us {

class Module;
class ServiceListenerEntryData;

/**
 * Data structure for saving service listener info. Contains
 * the optional service listener filter, in addition to the info
 * in ListenerEntry.
 */
class ServiceListenerEntry : public ServiceListenerHook::ListenerInfo
{

public:

  typedef US_SERVICE_LISTENER_FUNCTOR ServiceListener;

  ServiceListenerEntry(const ServiceListenerEntry& other);
  ServiceListenerEntry(const ServiceListenerHook::ListenerInfo& info);

  ~ServiceListenerEntry();
  ServiceListenerEntry& operator=(const ServiceListenerEntry& other);

  void SetRemoved(bool removed) const;

  ServiceListenerEntry(ModuleContext* mc, const ServiceListener& l, void* data, const std::string& filter = "");

  const LDAPExpr& GetLDAPExpr() const;

  LDAPExpr::LocalCache& GetLocalCache() const;

  void CallDelegate(const ServiceEvent& event) const;

  bool operator==(const ServiceListenerEntry& other) const;

  std::size_t Hash() const;

};

}

namespace std {
template<> struct hash<us::ServiceListenerEntry> { std::size_t operator()(const us::ServiceListenerEntry& arg) const {
  return arg.Hash();
} };
}

#endif // USSERVICELISTENERENTRY_H
