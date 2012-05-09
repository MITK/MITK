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
#include "../internal/berryCTKPluginActivator.h"

#include "Poco/Exception.h"

#include <iostream>

namespace berry {

QObject *ServiceRegistry::GetServiceFromCTK(const QString& serviceClass) const
{
  //BERRY_WARN << "Getting a CTK Service object through the BlueBerry service registry.\n"
  //              "You should use a ctkPluginContext or ctkServiceTracker instance instead!";

  ctkPluginContext* context = CTKPluginActivator::getPluginContext();
  if (context == 0)
  {
    // The org.blueberry.core_runtime plug-in was not started by the CTK Plugin Framework.
    // This is considered a fatal error.
    BERRY_FATAL << "The org.blueberry.core.runtime plug-in is not started. "
                   "Check that your application loads the correct provisioning "
                   "file and that it contains an entry for the org.blueberry.core.runtime plug-in.";
    return 0;
  }

  try
  {
    ctkServiceReference serviceRef = context->getServiceReference(serviceClass);
    return context->getService(serviceRef);
  }
  catch (const ctkServiceException& exc)
  {
    BERRY_INFO << exc.what();
  }

  return 0;
}

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
