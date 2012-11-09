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

#ifndef BERRYSERVICEREGISTRY_H_
#define BERRYSERVICEREGISTRY_H_

#include <org_blueberry_osgi_Export.h>

#include "berryService.h"

#include "Poco/Mutex.h"

#include <string>
#include <map>

namespace berry {

class BERRY_OSGI ServiceRegistry
{
private:
  std::map<const std::string, Service::Pointer> m_ServiceMap;

  mutable Poco::Mutex m_Mutex;

public:

  virtual ~ServiceRegistry();

  template<class S>
  typename S::Pointer GetServiceById(const std::string& id);

  void RegisterService(const std::string& id, Service::Pointer service);
  void UnRegisterService(const std::string& id);
  void UnRegisterService(Service::ConstPointer service);
};

}  // namespace berry

#include "berryServiceRegistry.txx"

#endif /*BERRYSERVICEREGISTRY_H_*/
