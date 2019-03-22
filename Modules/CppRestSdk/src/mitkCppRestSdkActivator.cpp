#include "mitkCppRestSdkActivator.h"
#include<usGetModuleContext.h>
#include<usModule.h> 
#include<usModuleContext.h> 
#include<usModuleInitialization.h> 
#include<usModuleResource.h> 
#include<usModuleResourceStream.h> 
#include<usModuleSettings.h> 
#include<mitkCommon.h>

void MitkCppRestSdkActivator::Load(us::ModuleContext *context)
{
  //Registration of the RESTManagerMicroservice
  m_RESTManager.reset(new mitk::RESTManager);
  us::ServiceProperties props;
  props[us::ServiceConstants::SERVICE_RANKING()] = 5;
  context->RegisterService<mitk::IRESTManager>(m_RESTManager.get(),props);
}

void MitkCppRestSdkActivator::Unload(us::ModuleContext *) 
{

}

US_EXPORT_MODULE_ACTIVATOR(MitkCppRestSdkActivator)
