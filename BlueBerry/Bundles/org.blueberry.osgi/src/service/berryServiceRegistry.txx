/*=========================================================================
 
Program:   BlueBerry Platform
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

#ifndef __BERRY_SERVICE_REGISTRY_TXX__
#define __BERRY_SERVICE_REGISTRY_TXX__

#include "../internal/berryCTKPluginActivator.h"
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
    ctkPluginContext* context = CTKPluginActivator::getPluginContext();
    if (context == 0)
    {
      // The org.blueberry.osgi plug-in was not started by the CTK Plugin Framework.
      // This is considered a fatal error.
      BERRY_FATAL << "The org.blueberry.osgi plug-in is not started. "
                     "Check that your application loads the correct provisioning "
                     "file and that it contains an entry for the org.blueberry.osgi plug-in.";
      return SmartPointer<S>();
    }

    try
    {
      ctkServiceReference serviceRef = context->getServiceReference<S>();
      if (!serviceRef) return SmartPointer<S>();

      S* service = context->getService<S>(serviceRef);
      if (!service)
      {
        return SmartPointer<S>();
      }
      //BERRY_WARN << "Getting a CTK Service object through the BlueBerry service registry.\n"
      //              "You should use a ctkPluginContext or ctkServiceTracker instance instead!";
      return typename S::Pointer(service);
    }
    catch (const ctkServiceException& exc)
    {
      BERRY_INFO << exc.what();
    }

    return SmartPointer<S>();
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
