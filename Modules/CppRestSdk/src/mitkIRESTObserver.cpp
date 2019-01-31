#include "mitkIRESTObserver.h"
#include <mitkCommon.h>
#include <mitkIRESTManager.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usServiceTracker.h>
mitk::IRESTObserver::~IRESTObserver() 
{
  us::ModuleContext *context = us::GetModuleContext();
  auto managerRef = context->GetServiceReference<IRESTManager>();
  if (managerRef)
  {
    auto managerService = context->GetService(managerRef);
    if (managerService)
    {
      managerService->handleDeleteClient();
    }
  }
  MITK_INFO << "virtual destructor was called";
}
