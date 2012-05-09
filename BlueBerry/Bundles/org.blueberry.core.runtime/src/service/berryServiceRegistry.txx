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

#ifndef __BERRY_SERVICE_REGISTRY_TXX__
#define __BERRY_SERVICE_REGISTRY_TXX__

#include "../berryLog.h"

#include <ctkPluginContext.h>

namespace berry {

template<class S>
typename S::Pointer ServiceRegistry::GetServiceById(const std::string& id)
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  Service::Pointer servicePtr;

  std::map<const std::string, Service::Pointer>::const_iterator serviceIt =
      m_ServiceMap.find(id);

  if (serviceIt != m_ServiceMap.end())
  {
    servicePtr = serviceIt->second;
  }

  if (servicePtr.IsNull())
  {
    // Try to get the service from the CTK Service Registry
    SmartPointer<S> ctkService(qobject_cast<S*>(GetServiceFromCTK(qobject_interface_iid<S*>())));
    return ctkService;
  }

  if (servicePtr->IsA(typeid(S)))
  {
    SmartPointer<S> castService = servicePtr.Cast<S>();
    return castService;
  }
  else throw Poco::BadCastException("The service could not be cast to: ", typeid(S).name());
}

} // namespace berry

#endif // __BERRY_SERVICE_REGISTRY_TXX__
