#include "mitkRESTServerMicroService.h"
#include <mitkCommon.h>
#include <usGetModuleContext.h>
#include <usModule.h>

mitk::RESTServerMicroService::RESTServerMicroService(web::uri uri) : m_Listener(uri)
{
  m_Uri = uri;
  m_Listener.support(MitkRESTMethods::GET, std::bind(&RESTServerMicroService::HandleGet, this, std::placeholders::_1));
  openListener();
}

mitk::RESTServerMicroService::~RESTServerMicroService()
{
  closeListener();
}

pplx::task<void> mitk::RESTServerMicroService::openListener()
{
  return m_Listener.open();
}

pplx::task<void> mitk::RESTServerMicroService::closeListener()
{
  return m_Listener.close();
}

web::uri mitk::RESTServerMicroService::GetUri() 
{
  return m_Uri;
}

void mitk::RESTServerMicroService::HandleGet(MitkRequest request)
{
  int port = m_Listener.uri().port();
 //getting exact request uri has to be a parameter in handle function
  web::uri_builder build(m_Listener.uri());
  build.append(request.absolute_uri());
  utility::string_t uri = build.to_uri().to_string();

  std::string uriString(uri.begin(), uri.end());
  web::json::value content;
  //example content, has to be replace by getting actual modified data by handle
  content[L"key 1"] = web::json::value::string(U("this is a first test"));
  request.set_body(content);
  auto answer = request.extract_json().get();
  MITK_INFO << "Test for Server at port " << port << " Exact request uri: " << uriString;
  
  //TODO replace worked by data object
  bool worked = false;
  us::ModuleContext *context = us::GetModuleContext();
  auto managerRef = context->GetServiceReference<IRESTManager>();
  if (managerRef)
  {
    auto managerService = context->GetService(managerRef);
    if (managerService)
    {
      worked = managerService->handle(build.to_uri());
    }
  }
  if (worked)
  {
    //TODO return modified data
    MITK_INFO << "Pipeline worked";
  }

  request.reply(MitkRestStatusCodes::OK, answer);
}