#ifndef MITKCPPRESTSDKACTIVATOR_H_
#define MITKCPPRESTSDKACTIVATOR_H_

#include <mitkRESTManager.h>

#include <usModuleActivator.h>
#include <usModuleEvent.h> 
#include <usServiceRegistration.h> 
#include <usServiceTracker.h>
#include <mitkRESTTest.h>
#include <mitkRESTTest2.h>

class MitkCppRestSdkActivator : public us::ModuleActivator
{
public:
  void Load(us::ModuleContext *context) override;
  void Unload(us::ModuleContext *) override;

private:
  std::unique_ptr<mitk::RESTManager> m_RESTManager;
  mitk::RestTest m_Test;
  mitk::RestTest2 m_Test2;

};
#endif
