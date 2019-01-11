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
  m_RESTClientMicroService.reset(new mitk::RESTClientMicroService);
  context->RegisterService<mitk::IRESTClient>(m_RESTClientMicroService.get());

  m_RESTServerMicroService.reset(new mitk::RESTServerMicroService);
  context->RegisterService<mitk::IRESTServer>(m_RESTServerMicroService.get());


  us::ServiceReference<mitk::IRESTClient> refClient = context->GetServiceReference<mitk::IRESTClient>();
  if (refClient)
  {
    auto serviceClient = context->GetService(refClient);
    if (serviceClient)
    {
      serviceClient->TestFunctionClient();
    }
  }


  us::ServiceReference<mitk::IRESTServer> refServer = context->GetServiceReference<mitk::IRESTServer>();
  if (refServer)
  {
    auto serviceServer = context->GetService(refServer);
    if (serviceServer)
    {
      serviceServer->TestFunctionServer();
    }
  }

}

void MitkCppRestSdkActivator::Unload(us::ModuleContext *) 
{

}

US_EXPORT_MODULE_ACTIVATOR(MitkCppRestSdkActivator)
