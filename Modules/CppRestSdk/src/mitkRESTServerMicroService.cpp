#include "mitkRESTServerMicroService.h"
#include <mitkCommon.h>

mitk::RESTServerMicroService::RESTServerMicroService(web::uri uri) : m_Listener(uri)
{
  m_Listener.support(MitkRESTMethods::GET, std::bind(&RESTServerMicroService::HandleGet, this, std::placeholders::_1));
  openListener();  
}

pplx::task<void> mitk::RESTServerMicroService::openListener()
{
  return m_Listener.open();
}

mitk::RESTServerMicroService::~RESTServerMicroService() 
{
}

void mitk::RESTServerMicroService::HandleGet(MitkRequest request)
{
  MITK_INFO << "Test for Server";
  request.reply(MitkRestStatusCodes::OK);
}