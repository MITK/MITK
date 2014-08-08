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

#include <mitkIPropertyAliases.h>
#include <mitkIPropertyDescriptions.h>
#include <mitkIPropertyExtensions.h>
#include <mitkIPropertyFilters.h>
#include <mitkIShaderRepository.h>     
#include <mitkISurfaceCutterFactory.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usServiceReference.h>
#include <usServiceTracker.h>

#include <itkSimpleFastMutexLock.h>
#include <itkMutexLockHolder.h>

namespace mitk {

static itk::SimpleFastMutexLock s_ContextToServicesMapMutex;
static std::map<us::ModuleContext*, std::map<void*,us::ServiceReferenceU> > s_ContextToServicesMap;

template<class S>
static S* GetCoreService(us::ModuleContext* context)
{
  itk::MutexLockHolder<itk::SimpleFastMutexLock> l(s_ContextToServicesMapMutex);
  S* coreService = NULL;
  us::ServiceReference<S> serviceRef = context->GetServiceReference<S>();
  if (serviceRef)
  {
    coreService = context->GetService(serviceRef);
  }

  assert(coreService && "Asserting non-NULL MITK core service");
  s_ContextToServicesMap[context].insert(std::make_pair(coreService,serviceRef));

  return coreService;
}

IShaderRepository* CoreServices::GetShaderRepository()
{
  static us::ServiceTracker<IShaderRepository> tracker(us::GetModuleContext());
  tracker.Open();
  return tracker.GetService();
}

IPropertyAliases* CoreServices::GetPropertyAliases(us::ModuleContext* context)
{
  return GetCoreService<IPropertyAliases>(context);
}

IPropertyDescriptions* CoreServices::GetPropertyDescriptions(us::ModuleContext* context)
{
  return GetCoreService<IPropertyDescriptions>(context);
}

IPropertyExtensions* CoreServices::GetPropertyExtensions(us::ModuleContext* context)
{
  return GetCoreService<IPropertyExtensions>(context);
}

IPropertyFilters* CoreServices::GetPropertyFilters(us::ModuleContext* context)
{
  return GetCoreService<IPropertyFilters>(context);
}

ISurfaceCutterFactory* CoreServices::GetSurfaceCutterFactory(us::ModuleContext* context)
{
    return GetCoreService<ISurfaceCutterFactory>(context);
}

bool CoreServices::Unget(us::ModuleContext* context, const std::string& /*interfaceId*/, void* service)
{
  bool success = false;

  itk::MutexLockHolder<itk::SimpleFastMutexLock> l(s_ContextToServicesMapMutex);
  std::map<us::ModuleContext*, std::map<void*,us::ServiceReferenceU> >::iterator iter = s_ContextToServicesMap.find(context);
  if (iter != s_ContextToServicesMap.end())
  {
    std::map<void*,us::ServiceReferenceU>::iterator iter2 = iter->second.find(service);
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
