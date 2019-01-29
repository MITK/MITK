#include "mitkRESTTest2.h"
#include <mitkCommon.h>
#include <usGetModuleContext.h>
#include <usModule.h>

mitk::RestTest2::RestTest2() {}

mitk::RestTest2::~RestTest2() {}

void mitk::RestTest2::TestRESTServer()
{
  us::ModuleContext *context = us::GetModuleContext();
  auto managerRef = context->GetServiceReference<IRESTManager>();
  if (managerRef)
  {
    auto managerService = context->GetService(managerRef);
    if (managerService)
    {
      managerService->receiveRequest(L"http://localhost:8080/example", this);
      managerService->receiveRequest(L"http://localhost:8090", this);
    }
  }
}

// TODO get Data, return modified Data
web::json::value mitk::RestTest2::notify(web::json::value data)
{
  MITK_INFO << "Observer: Data in observer";
  data[L"key 1"] = web::json::value::string(U("Data modified by /example and port 8090 observer"));
  return data;
}