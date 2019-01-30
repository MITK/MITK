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
      managerService->receiveRequest(L"http://localhost:8080/test", this);
    }
  }
}

//TODO get Data, return modified Data
web::json::value mitk::RestTest::notify(web::json::value data) 
{
  MITK_INFO << "Observer: Data in observer";
  //data[L"key 1"] = web::json::value::string(U("Data modified by /test Observer"));
  return data.at(U("key 1"));
}
