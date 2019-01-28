#include "mitkRESTTest.h"
#include <usGetModuleContext.h>
#include <usModule.h>
#include <mitkCommon.h>

mitk::RestTest::RestTest() {}

mitk::RestTest::~RestTest() {}

void mitk::RestTest::TestRESTServer()
{
  us::ModuleContext *context = us::GetModuleContext();
  auto managerRef = context->GetServiceReference<IRESTManager>();
  if (managerRef)
  {
    auto managerService = context->GetService(managerRef);
    if (managerService)
    {
      managerService->receiveRequest(L"http://localhost:8080/test/", this);
      managerService->receiveRequest(L"http://localhost:8080/example", this);
      managerService->receiveRequest(L"http://localhost:8090", this);
    }
  }
}

//TODO get Data, return modified Data
bool mitk::RestTest::notify() 
{
  return true;
}
