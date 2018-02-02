/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkCoreServices.h"

#include <mitkIMimeTypeProvider.h>
#include <mitkIPropertyAliases.h>
#include <mitkIPropertyDescriptions.h>
#include <mitkIPropertyExtensions.h>
#include <mitkIPropertyFilters.h>
#include <mitkIPropertyPersistence.h>
#include <mitkIPropertyRelations.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usServiceReference.h>
#include <usServiceTracker.h>

#include <itkMutexLockHolder.h>
#include <itkSimpleFastMutexLock.h>

namespace mitk
{
  itk::SimpleFastMutexLock &s_ContextToServicesMapMutex()
  {
    static itk::SimpleFastMutexLock mutex;
    return mutex;
  }

  std::map<us::ModuleContext *, std::map<void *, us::ServiceReferenceU>> &s_ContextToServicesMap()
  {
    static std::map<us::ModuleContext *, std::map<void *, us::ServiceReferenceU>> serviceMap;
    return serviceMap;
  }

  template <class S>
  static S *GetCoreService(us::ModuleContext *context)
  {
    if (context == nullptr)
      context = us::GetModuleContext();

    S *coreService = nullptr;
    us::ServiceReference<S> serviceRef = context->GetServiceReference<S>();
    if (serviceRef)
    {
      coreService = context->GetService(serviceRef);
    }

    assert(coreService && "Asserting non-nullptr MITK core service");
    {
      itk::MutexLockHolder<itk::SimpleFastMutexLock> l(s_ContextToServicesMapMutex());
      s_ContextToServicesMap()[context].insert(std::make_pair(coreService, serviceRef));
    }

    return coreService;
  }

  IPropertyAliases *CoreServices::GetPropertyAliases(us::ModuleContext *context)
  {
    return GetCoreService<IPropertyAliases>(context);
  }

  IPropertyDescriptions *CoreServices::GetPropertyDescriptions(us::ModuleContext *context)
  {
    return GetCoreService<IPropertyDescriptions>(context);
  }

  IPropertyExtensions *CoreServices::GetPropertyExtensions(us::ModuleContext *context)
  {
    return GetCoreService<IPropertyExtensions>(context);
  }

  IPropertyFilters *CoreServices::GetPropertyFilters(us::ModuleContext *context)
  {
    return GetCoreService<IPropertyFilters>(context);
  }

  IPropertyPersistence *CoreServices::GetPropertyPersistence(us::ModuleContext *context)
  {
    return GetCoreService<IPropertyPersistence>(context);
  }

  IPropertyRelations *CoreServices::GetPropertyRelations(us::ModuleContext *context)
  {
    return GetCoreService<IPropertyRelations>(context);
  }

  IMimeTypeProvider *CoreServices::GetMimeTypeProvider(us::ModuleContext *context)
  {
    return GetCoreService<IMimeTypeProvider>(context);
  }

  bool CoreServices::Unget(us::ModuleContext *context, const std::string & /*interfaceId*/, void *service)
  {
    bool success = false;

    itk::MutexLockHolder<itk::SimpleFastMutexLock> l(s_ContextToServicesMapMutex());
    auto iter =
      s_ContextToServicesMap().find(context);
    if (iter != s_ContextToServicesMap().end())
    {
      auto iter2 = iter->second.find(service);
      if (iter2 != iter->second.end())
      {
        us::ServiceReferenceU serviceRef = iter2->second;
        if (serviceRef)
        {
          success = context->UngetService(serviceRef);
          if (success)
          {
            iter->second.erase(iter2);
          }
        }
      }
    }

    return success;
  }
}
