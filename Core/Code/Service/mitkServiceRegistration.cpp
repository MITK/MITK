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

#include "mitkServiceRegistration.h"
#include "mitkServiceRegistrationPrivate.h"
#include "mitkServiceListenerEntry_p.h"
#include "mitkServiceRegistry_p.h"
#include "mitkServiceFactory.h"

#include "mitkModulePrivate.h"
#include "mitkCoreModuleContext_p.h"

#include <mitkLogMacros.h>

#include <stdexcept>

namespace mitk {

typedef ServiceRegistrationPrivate::MutexLocker MutexLocker;

ServiceRegistration::ServiceRegistration()
  : d(0)
{

}

ServiceRegistration::ServiceRegistration(const ServiceRegistration& reg)
  : d(reg.d)
{
  if (d) d->ref.Ref();
}

ServiceRegistration::ServiceRegistration(ServiceRegistrationPrivate* registrationPrivate)
  : d(registrationPrivate)
{
  d->ref.Ref();
}

ServiceRegistration::ServiceRegistration(ModulePrivate* module, itk::LightObject* service,
                                         const ServiceProperties& props)
  : d(new ServiceRegistrationPrivate(module, service, props))
{

}

ServiceRegistration::operator bool() const
{
  return d != 0;
}

ServiceRegistration& ServiceRegistration::operator=(int null)
{
  if (null == 0)
  {
    if (d && !d->ref.Deref())
    {
      delete d;
    }
    d = 0;
  }
  return *this;
}

ServiceRegistration::~ServiceRegistration()
{
  if (d && !d->ref.Deref())
    delete d;
}

ServiceReference ServiceRegistration::GetReference() const
{
  if (!d) throw std::logic_error("ServiceRegistration object invalid");
  if (!d->available) throw std::logic_error("Service is unregistered");

  return d->reference;
}

void ServiceRegistration::SetProperties(const ServiceProperties& props)
{
  if (!d) throw std::logic_error("ServiceRegistration object invalid");

  MutexLocker lock(d->eventLock);

  ServiceListeners::ServiceListenerEntries before;
  // TBD, optimize the locking of services
  {
    //MutexLocker lock2(d->module->coreCtx->globalFwLock);
    MutexLocker lock3(d->propsLock);

    if (d->available)
    {
      // NYI! Optimize the MODIFIED_ENDMATCH code
      int old_rank = any_cast<int>(d->properties[ServiceConstants::SERVICE_RANKING()]);
      d->module->coreCtx->listeners.GetMatchingServiceListeners(d->reference, before);
      const std::list<std::string>& classes = ref_any_cast<std::list<std::string> >(d->properties[ServiceConstants::OBJECTCLASS()]);
      long int sid = any_cast<long int>(d->properties[ServiceConstants::SERVICE_ID()]);
      d->properties = ServiceRegistry::CreateServiceProperties(props, classes, sid);
      int new_rank = any_cast<int>(d->properties[ServiceConstants::SERVICE_RANKING()]);
      if (old_rank != new_rank)
      {
        d->module->coreCtx->services.UpdateServiceRegistrationOrder(*this, classes);
      }
    }
    else
    {
      throw std::logic_error("Service is unregistered");
    }
  }
  ServiceListeners::ServiceListenerEntries matchingListeners;
  d->module->coreCtx->listeners.GetMatchingServiceListeners(d->reference, matchingListeners, false);
  d->module->coreCtx->listeners.ServiceChanged(matchingListeners,
                                               ServiceEvent(ServiceEvent::MODIFIED, d->reference),
                                               before);

  d->module->coreCtx->listeners.ServiceChanged(before,
                                               ServiceEvent(ServiceEvent::MODIFIED_ENDMATCH, d->reference));
}

void ServiceRegistration::Unregister()
{
  if (!d) throw std::logic_error("ServiceRegistration object invalid");

  if (d->unregistering) return; // Silently ignore redundant unregistration.
  {
    MutexLocker lock(d->eventLock);
    if (d->unregistering) return;
    d->unregistering = true;

    if (d->available)
    {
      if (d->module)
      {
        d->module->coreCtx->services.RemoveServiceRegistration(*this);
      }
    }
    else
    {
      throw std::logic_error("Service is unregistered");
    }
  }

  if (d->module)
  {
    ServiceListeners::ServiceListenerEntries listeners;
    d->module->coreCtx->listeners.GetMatchingServiceListeners(d->reference, listeners);
     d->module->coreCtx->listeners.ServiceChanged(
         listeners,
         ServiceEvent(ServiceEvent::UNREGISTERING, d->reference));
  }

  {
    MutexLocker lock(d->eventLock);
    {
      MutexLocker lock2(d->propsLock);
      d->available = false;
      if (d->module)
      {
        ServiceRegistrationPrivate::ModuleToServicesMap::const_iterator end = d->serviceInstances.end();
        for (ServiceRegistrationPrivate::ModuleToServicesMap::const_iterator i = d->serviceInstances.begin();
             i != end; ++i)
        {
          itk::LightObject* obj = i->second;
          try
          {
            // NYI, don't call inside lock
            dynamic_cast<ServiceFactory*>(d->service)->UngetService(i->first,
                                                                    *this,
                                                                    obj);
          }
          catch (const std::exception& /*ue*/)
          {
            MITK_WARN << "mitk::ServiceFactory UngetService implementation threw an exception";
          }
        }
      }
      d->module = 0;
      d->dependents.clear();
      d->service = 0;
      d->serviceInstances.clear();;
      d->unregistering = false;
    }
  }
}

bool ServiceRegistration::operator<(const ServiceRegistration& o) const
{
  if (!d) return true;
  return d->reference <(o.d->reference);
}

bool ServiceRegistration::operator==(const ServiceRegistration& registration) const
{
  return d == registration.d;
}

ServiceRegistration& ServiceRegistration::operator=(const ServiceRegistration& registration)
{
  ServiceRegistrationPrivate* curr_d = d;
  d = registration.d;
  d->ref.Ref();

  if (curr_d && !curr_d->ref.Deref())
    delete curr_d;

  return *this;
}

}
