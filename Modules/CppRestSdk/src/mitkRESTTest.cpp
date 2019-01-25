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
      managerService->receiveRequest(L"http://localhost:8080/test");
      managerService->receiveRequest(L"http://localhost:8080/example");
      managerService->receiveRequest(L"http://localhost:8090");
    }
  }
}

bool mitk::RestTest::notify() 
{
  return true;
}
