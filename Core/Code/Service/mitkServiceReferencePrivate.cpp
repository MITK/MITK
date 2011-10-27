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

#include "mitkServiceReferencePrivate.h"

#include "mitkServiceFactory.h"
#include "mitkServiceException.h"
#include "mitkServiceRegistry_p.h"
#include "mitkServiceRegistrationPrivate.h"

#include "mitkModule.h"
#include "mitkModulePrivate.h"
#include "mitkCoreModuleContext_p.h"

#include <mitkLogMacros.h>

#include <itkMutexLockHolder.h>

namespace mitk {

typedef ServiceRegistrationPrivate::MutexLocker MutexLocker;

ServiceReferencePrivate::ServiceReferencePrivate(ServiceRegistrationPrivate* reg)
  : ref(1), registration(reg)
{
}

itk::LightObject* ServiceReferencePrivate::GetService(Module* module)
{
  itk::LightObject* s = 0;
  {
    MutexLocker lock(registration->propsLock);
    if (registration->available)
    {
      int count = registration->dependents[module];
      if (count == 0)
      {
        const std::list<std::string>& classes =
            ref_any_cast<std::list<std::string> >(registration->properties[ServiceConstants::OBJECTCLASS()]);
        registration->dependents[module] = 1;
        if (ServiceFactory* serviceFactory = dynamic_cast<ServiceFactory*>(registration->GetService()))
        {
          try
          {
            s = serviceFactory->GetService(module, ServiceRegistration(registration));
          }
          catch (...)
          {
            MITK_WARN << "mitk::ServiceFactory threw an exception";
            return 0;
          }
          if (s == 0) {
            MITK_WARN << "mitk::ServiceFactory produced null";
            return 0;
          }
          for (std::list<std::string>::const_iterator i = classes.begin();
               i != classes.end(); ++i)
          {
            if (!registration->module->coreCtx->services.CheckServiceClass(s, *i))
            {
              MITK_WARN << "mitk::ServiceFactory produced an object "
                           "that did not implement: " << (*i);
              return 0;
            }
          }
          registration->serviceInstances.insert(std::make_pair(module, s));
        }
        else
        {
          s = registration->GetService();
        }
      }
      else
      {
        registration->dependents.insert(std::make_pair(module, count + 1));
        if (dynamic_cast<ServiceFactory*>(registration->GetService()))
        {
          s = registration->serviceInstances[module];
        }
        else
        {
          s = registration->GetService();
        }
      }
    }
  }
  return s;
}

bool ServiceReferencePrivate::UngetService(Module* module, bool checkRefCounter)
{
  MutexLocker lock(registration->propsLock);
  bool hadReferences = false;
  bool removeService = false;

  int count= registration->dependents[module];
  if (count > 0)
  {
    hadReferences = true;
  }

  if(checkRefCounter)
  {
    if (count > 1)
    {
      registration->dependents[module] = count - 1;
    }
    else if(count == 1)
    {
      removeService = true;
    }
  }
  else
  {
    removeService = true;
  }

  if (removeService)
  {
    itk::LightObject* sfi = registration->serviceInstances[module];
    registration->serviceInstances.erase(module);
    if (sfi != 0)
    {
      try
      {
        dynamic_cast<ServiceFactory*>(
              registration->GetService())->UngetService(module, ServiceRegistration(registration), sfi);
      }
      catch (const std::exception& /*e*/)
      {
        MITK_WARN << "mitk::ServiceFactory threw an exception";
      }
    }
    registration->dependents.erase(module);
  }

  return hadReferences;
}

ServiceProperties ServiceReferencePrivate::GetProperties() const
{
  return registration->properties;
}

Any ServiceReferencePrivate::GetProperty(const std::string& key, bool lock) const
{
  if (lock)
  {
    MutexLocker lock(registration->propsLock);
    ServiceProperties::const_iterator iter = registration->properties.find(key);
    if (iter != registration->properties.end())
      return iter->second;
  }
  else
  {
    ServiceProperties::const_iterator iter = registration->properties.find(key);
    if (iter != registration->properties.end())
      return iter->second;
  }
  return Any();
}

}
