#include "mitkRESTManager.h"
#include <mitkCommon.h>

mitk::RESTManager::RESTManager() {}

mitk::RESTManager::~RESTManager() {}

void mitk::RESTManager::sendRequest(RequestType type) 
{
  switch (type)
  {
    case get:
      //Call get in mitkRESTClientMicroService
      break;
    case post:
      //Call post in mitkRESTClientMicroService
      break;
    case put:
      //Call put in mitkRESTClientMicroService
      break;
  }
}

void mitk::RESTManager::receiveRequest(web::uri uri) 
{
  //New instance of RESTServerMicroservice in m_ServerMap, key is port of the request
  int port = uri.port();
  RESTServerMicroService* server = new RESTServerMicroService(uri);
  m_ServerMap[port] = server;
  
  MITK_INFO <<port<<":"<< m_ServerMap.count(port);
}
