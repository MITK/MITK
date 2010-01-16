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

#include "berryServiceUse.h"

#include <supplement/framework.debug/berryDebug.h>

#include "../osgi.framework/berryBundleContextImpl.h"
#include "../osgi.framework/berryBundleHost.h"
#include "berryServiceRegistrationImpl.h"
#include "berryServiceRegistry.h"

namespace berry {
namespace osgi {
namespace internal {

ServiceUse::ServiceUse(SmartPointer<BundleContextImpl> context,
      SmartPointer<ServiceRegistrationImpl> registration)
: context(context), registration(registration), useCount(0)
{
    Object::Pointer service = registration->GetServiceObject();
    if (service.Cast<ServiceFactory>()) {
      factory = service.Cast<ServiceFactory>();
    } else {
      cachedService = service;
    }
  }

  Object::Pointer ServiceUse::GetService() {
    if ((useCount > 0) || !factory) {
      useCount++;
      return cachedService;
    }

    if (Debug::DEBUG && Debug::DEBUG_SERVICES) {
      std::cout << "getService[factory=" << registration->GetBundle() << "](" << context->GetBundleImpl() << "," << registration << ")" << std::endl;
    }
    Object::Pointer service;
    try {
      service = factory->GetService(context->GetBundleImpl(), registration);
    }
    catch (const std::exception* e)
    {
      if (Debug::DEBUG && Debug::DEBUG_SERVICES) {
        std::cout << factory->GetClassName() << "::GetService() exception: " << e->what() << std::endl;
        Debug::PrintStackTrace();
      }
      //TODO allow the adaptor to handle this unexpected error
      //context.getFramework().getAdaptor().handleRuntimeError(t);
      //ServiceException se = new ServiceException(NLS.bind(Msg.SERVICE_FACTORY_EXCEPTION, factory.getClass().getName(), "getService"), ServiceException.FACTORY_EXCEPTION, t); //$NON-NLS-1$
      //context.getFramework().publishFrameworkEvent(FrameworkEvent.ERROR, registration.getBundle(), se);
      return Object::Pointer(0);
    }

    if (!service) {
      if (Debug::DEBUG && Debug::DEBUG_SERVICES) {
        std::cout << factory->GetClassName() << "::GetService() returned 0." << std::endl;
      }

      //TODO ServiceException & event
      //ServiceException se = new ServiceException(NLS.bind(Msg.SERVICE_OBJECT_NULL_EXCEPTION, factory.getClass().getName()), ServiceException.FACTORY_ERROR);
      //context.getFramework().publishFrameworkEvent(FrameworkEvent.ERROR, registration.getBundle(), se);
      return Object::Pointer(0);
    }

    std::vector<std::string> clazzes(registration->GetClasses());
    std::string invalidService = ServiceRegistry::CheckServiceClass(clazzes, service);
    if (!invalidService.empty()) {
      if (Debug::DEBUG && Debug::DEBUG_SERVICES) {
        std::cout << "Service object is not an instance of " << invalidService << std::endl;
      }
      //TODO ServiceException & event
      //ServiceException se = new ServiceException(NLS.bind(Msg.SERVICE_FACTORY_NOT_INSTANCEOF_CLASS_EXCEPTION, factory.getClass().getName(), invalidService), ServiceException.FACTORY_ERROR);
      //context.getFramework().publishFrameworkEvent(FrameworkEvent.ERROR, registration.getBundle(), se);
      return Object::Pointer(0);
    }

    this->cachedService = service;
    useCount++;

    return service;
  }

  bool ServiceUse::UngetService() {
    if (useCount == 0) {
      return true;
    }

    useCount--;
    if (useCount > 0) {
      return false;
    }

    if (!factory) {
      return true;
    }

    const Object::Pointer service = cachedService;
    cachedService = 0;

    if (Debug::DEBUG && Debug::DEBUG_SERVICES) {
      std::cout << "UngetService[factory=" << registration->GetBundle() << "](" << context->GetBundleImpl() << "," << registration << ")" << std::endl;
    }
    try {
      factory->UngetService(context->GetBundleImpl(), registration, service);
    } catch (const std::exception& e)
    {
      if (Debug::DEBUG && Debug::DEBUG_GENERAL) {
        std::cout << factory << "::UngetService() exception" << std::endl;
        Debug::PrintStackTrace();
      }

      //TODO ServiceException & event
      //ServiceException se = new ServiceException(NLS.bind(Msg.SERVICE_FACTORY_EXCEPTION, factory.getClass().getName(), "ungetService"), ServiceException.FACTORY_EXCEPTION, t); //$NON-NLS-1$
      //context.getFramework().publishFrameworkEvent(FrameworkEvent.ERROR, registration.getBundle(), se);
    }

    return true;
  }

  void ServiceUse::ReleaseService() {
    if ((useCount == 0) || !factory) {
      return;
    }
    const Object::Pointer service = cachedService;
    cachedService = 0;
    useCount = 0;

    if (Debug::DEBUG && Debug::DEBUG_SERVICES) {
      std::cout << "ReleaseService[factory=" << registration->GetBundle() << "](" << context->GetBundleImpl() << "," << registration << ")" << std::endl;
    }
    try {
      factory->UngetService(context->GetBundleImpl(), registration, service);
    } catch (const std::exception& e) {
      if (Debug::DEBUG && Debug::DEBUG_SERVICES) {
        std::cout << factory << "::UngetService() exception: " << e.what() << std::endl;
        Debug::PrintStackTrace();
      }

      //TODO ServiceException & event
      //ServiceException se = new ServiceException(NLS.bind(Msg.SERVICE_FACTORY_EXCEPTION, factory.getClass().getName(), "ungetService"), ServiceException.FACTORY_EXCEPTION, t); //$NON-NLS-1$
      //context.getFramework().publishFrameworkEvent(FrameworkEvent.ERROR, registration.getBundle(), se);
    }
  }

}
}
}
