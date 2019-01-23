#include "mitkRESTServerMicroService.h"
#include <mitkCommon.h>

mitk::RESTServerMicroService::RESTServerMicroService(web::uri uri) : m_Listener(uri)
{
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

void mitk::RESTServerMicroService::HandleGet(MitkRequest request)
{
  int port = m_Listener.uri().port();
  MITK_INFO << "Test for Server at port "<<port;
  request.reply(MitkRestStatusCodes::OK);
}