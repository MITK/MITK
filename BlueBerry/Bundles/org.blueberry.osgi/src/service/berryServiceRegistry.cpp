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

#include "berryLog.h"

#include "berryServiceRegistry.h"

#include "Poco/Exception.h"

#include <iostream>

namespace berry {

ServiceRegistry::~ServiceRegistry()
{

}

void ServiceRegistry::RegisterService(const std::string& id, Service::Pointer service)
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  if (m_ServiceMap.find(id) != m_ServiceMap.end())
    throw Poco::ExistsException("The following service is already registered:", id);

  m_ServiceMap[id] = service;
  //BERRY_INFO << "Service " << id << " registered\n";
}

void ServiceRegistry::UnRegisterService(const std::string& /*id*/)
{
  throw Poco::NotImplementedException();
}

void ServiceRegistry::UnRegisterService(Service::ConstPointer /*service*/)
{
  throw Poco::NotImplementedException();
}

} // namespace berry
