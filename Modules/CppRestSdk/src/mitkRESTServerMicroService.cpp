#include "mitkRESTServerMicroService.h"
#include <mitkCommon.h>

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

  web::uri_builder build(m_Listener.uri());
  build.append(request.absolute_uri());
  utility::string_t uri = build.to_uri().to_string();

  std::string uriString(uri.begin(), uri.end());
  web::json::value content;

  content[L"key 1"] = web::json::value::string(U("this is a first test"));
  request.set_body(content);
  auto answer = request.extract_json().get();
  MITK_INFO << "Test for Server at port " << port << " Exact request uri: " << uriString;

  request.reply(MitkRestStatusCodes::OK, answer);
}