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

void mitk::RESTManager::receiveRequest(web::uri uri, mitk::IRESTObserver *observer)
{
  //New instance of RESTServerMicroservice in m_ServerMap, key is port of the request
  int port = uri.port();
  
  if (m_ServerMap.count(port) == 0)
  {
    //new observer has to be added
    m_Observer[uri] = observer;

    //testing if entry has been added to map
    utility::string_t uristringt = uri.to_string();
    std::string uristring(uristringt.begin(), uristringt.end());
    MITK_INFO << uristring << " : Number of elements in map: " << m_Observer.count(uri);

    //creating server instance
    RESTServerMicroService *server = new RESTServerMicroService(uri.authority());
    //add reference to server instance to map
    m_ServerMap[port] = server;

    //info output
    utility::string_t host = uri.authority().to_string();
    std::string hoststring(host.begin(), host.end());
    MITK_INFO << "new server" << hoststring<<" at port" << port;
  }
  else
  {
    if (m_ServerMap[port]->GetUri() == uri.authority())
    {
      //new observer has to be added
      m_Observer[uri] = observer;

      // testing if entry has been added to map
      utility::string_t uristringt = uri.to_string();
      std::string uristring(uristringt.begin(), uristringt.end());
      MITK_INFO << uristring << " : Number of elements in map: " << m_Observer.count(uri);

      //info output
      MITK_INFO << "started listening, no new server instance has been created";
    }
    else
    {
      MITK_ERROR << "there is already another server listening under this port";
    }
  }
}
