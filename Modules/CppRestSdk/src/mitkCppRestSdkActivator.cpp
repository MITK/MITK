#include "mitkCppRestSdkActivator.h"

#include <usGetModuleContext.h>
#include<usModule.h> 
#include<usModuleActivator.h> 
#include<usModuleContext.h> 
#include<usModuleEvent.h> 
#include<usModuleInitialization.h> 
#include<usModuleResource.h> 
#include<usModuleResourceStream.h> 
#include<usModuleSettings.h> 
#include<usServiceTracker.h>


void MitkCppRestSdkActivator::Load(us::ModuleContext *context)
{
  //Registration of the RESTManagerMicroservice
  m_RESTManager.reset(new mitk::RESTManager);
  context->RegisterService<mitk::IRESTManager>(m_RESTManager.get());

  //Test use of the RESTManagerMicroservice, later done in Module
  us::ServiceReference<mitk::IRESTManager> refManager = context->GetServiceReference<mitk::IRESTManager>();
  if (refManager)
  {
    auto serviceClient = context->GetService(refManager);
    if (serviceClient)
    {
      serviceClient->receiveRequest(L"http://localhost:8080");
      serviceClient->receiveRequest(L"http://localhost:8090");
    }
  }
}

void MitkCppRestSdkActivator::Unload(us::ModuleContext *) 
{

}

US_EXPORT_MODULE_ACTIVATOR(MitkCppRestSdkActivator)
