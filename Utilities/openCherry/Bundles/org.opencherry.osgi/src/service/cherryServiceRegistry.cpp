/*=========================================================================
 
Program:   openCherry Platform
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

#include "cherryServiceRegistry.h"

#include "Poco/Exception.h"

#include <iostream>

namespace cherry {

ServiceRegistry::~ServiceRegistry()
{
  
}

void ServiceRegistry::RegisterService(const std::string& id, Service::Pointer service)
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  if (m_ServiceMap.find(id) != m_ServiceMap.end())
    throw Poco::ExistsException("The following service is already registered:", id);
  
  m_ServiceMap[id] = service;
  std::cout << "Service " << id << " registered\n";
}

void ServiceRegistry::UnRegisterService(const std::string& /*id*/)
{
  throw Poco::NotImplementedException();
}

void ServiceRegistry::UnRegisterService(Service::ConstPointer /*service*/)
{
  throw Poco::NotImplementedException();
}

} // namespace cherry
