#ifndef MITKCPPRESTSDKACTIVATOR_H_
#define MITKCPPRESTSDKACTIVATOR_H_

#include <mitkRESTClientMicroService.h>
#include <mitkRESTServerMicroService.h>

#include <usModuleActivator.h>
#include <usModuleEvent.h> 
#include <usServiceRegistration.h> 
#include <usServiceTracker.h>

class MitkCppRestSdkActivator : public us::ModuleActivator
{
public:
  void Load(us::ModuleContext *context) override;
  void Unload(us::ModuleContext *) override;

private:
  std::unique_ptr<mitk::RESTClientMicroService> m_RESTClientMicroService;
  std::unique_ptr<mitk::RESTServerMicroService> m_RESTServerMicroService;

};
#endif
