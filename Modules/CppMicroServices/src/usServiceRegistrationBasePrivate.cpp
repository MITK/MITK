/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "usServiceRegistrationBasePrivate.h"

namespace us {

ServiceRegistrationBasePrivate::ServiceRegistrationBasePrivate(
  ModulePrivate* module, const InterfaceMap& service,
  const ServicePropertiesImpl& props)
  : ref(0), service(service), module(module), reference(this),
    properties(props), available(true), unregistering(false)
{
  // The reference counter is initialized to 0 because it will be
  // incremented by the "reference" member.
}

ServiceRegistrationBasePrivate::~ServiceRegistrationBasePrivate()
{

}

bool ServiceRegistrationBasePrivate::IsUsedByModule(Module* p) const
{
  return (dependents.find(p) != dependents.end()) ||
      (prototypeServiceInstances.find(p) != prototypeServiceInstances.end());
}

const InterfaceMap& ServiceRegistrationBasePrivate::GetInterfaces() const
{
  return service;
}

void* ServiceRegistrationBasePrivate::GetService(const std::string& interfaceId) const
{
  if (interfaceId.empty() && service.size() > 0)
  {
    return service.begin()->second;
  }

  InterfaceMap::const_iterator iter = service.find(interfaceId);
  if (iter != service.end())
  {
    return iter->second;
  }
  return nullptr;
}

}
