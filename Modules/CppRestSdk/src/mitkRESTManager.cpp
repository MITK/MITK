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

  //Checking if port is free to add a new Server
  if (m_ServerMap.count(port) == 0)
  {
    //new observer has to be added
    m_Observer[uri] = observer;

    //testing if entry has been added to observer map
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
  //If there is already a server under this port
  else
  {
    //Same host, means new observer but not a new server instance
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
    //Error, since another server can't be added under this port
    else
    {
      MITK_ERROR << "there is already another server listening under this port";
    }
  }
}

//TODO replace boolean as return value by actual data
bool mitk::RESTManager::handle(web::uri uri)
{
  if (m_Observer.count(uri) == 1)
  {
    //TODO replace by data
    bool worked = m_Observer[uri]->notify();
    return worked;
  }
  else
  {
    MITK_WARN << "No Observer can handle the data";
    return false;
  }
}
