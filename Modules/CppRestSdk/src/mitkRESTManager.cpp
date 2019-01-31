#include "mitkRESTManager.h"
#include <mitkCommon.h>

mitk::RESTManager::RESTManager() {}

mitk::RESTManager::~RESTManager() {}

void mitk::RESTManager::sendRequest(RequestType type)
{
  switch (type)
  {
    case get:
      // Call get in mitkRESTClientMicroService
      break;
    case post:
      // Call post in mitkRESTClientMicroService
      break;
    case put:
      // Call put in mitkRESTClientMicroService
      break;
  }
}

void mitk::RESTManager::receiveRequest(web::uri uri, mitk::IRESTObserver *observer)
{
  // New instance of RESTServerMicroservice in m_ServerMap, key is port of the request
  int port = uri.port();

  // Checking if port is free to add a new Server
  if (m_ServerMap.count(port) == 0)
  {
    // new observer has to be added
    std::pair<int, utility::string_t> key(uri.port(), uri.path());
    m_Observer[key] = observer;

    // testing if entry has been added to observer map
    utility::string_t uristringt = uri.path();
    std::string uristring(uristringt.begin(), uristringt.end());
    MITK_INFO << uristring << " : Number of elements in map: " << m_Observer.count(key);

    // creating server instance
    RESTServerMicroService *server = new RESTServerMicroService(uri.authority());
    // add reference to server instance to map
    m_ServerMap[port] = server;

    // info output
    utility::string_t host = uri.authority().to_string();
    std::string hoststring(host.begin(), host.end());
    MITK_INFO << "new server" << hoststring << " at port" << port;
  }
  // If there is already a server under this port
  else
  {
    // Same host, means new observer but not a new server instance
    if (m_ServerMap[port]->GetUri() == uri.authority())
    {
      // new observer has to be added
      std::pair<int, utility::string_t> key(uri.port(), uri.path());
      m_Observer[key] = observer;

      // testing if entry has been added to map
      utility::string_t uristringt = uri.to_string();
      std::string uristring(uristringt.begin(), uristringt.end());
      MITK_INFO << uristring << " : Number of elements in map: " << m_Observer.count(key);

      // info output
      MITK_INFO << "started listening, no new server instance has been created";
    }
    // Error, since another server can't be added under this port
    else
    {
      MITK_ERROR << "there is already another server listening under this port";
    }
  }
}


web::json::value mitk::RESTManager::handle(web::uri uri, web::json::value data)
{
  // Checking if is there is a observer for the port and path
  std::pair<int, utility::string_t> key(uri.port(), uri.path());
  if (m_Observer.count(key) != 0)
  {
    MITK_INFO << "Manager: Data send to observer";
    return m_Observer[key]->notify(data);
  }
  //No map under this port, delete Server Object to release port for other servers
  else
  {
    MITK_WARN << "No Observer can handle the data";
    return NULL;
  }
}

void mitk::RESTManager::handleDeleteClient(mitk::IRESTObserver *observer)
{
  for (auto it = m_Observer.begin(); it != m_Observer.end();)
  {
    mitk::IRESTObserver *obsMap = it->second;
    //Check weather observer is at this place in map
    if (obsMap == observer)
    {

      int port = it->first.first;
      utility::string_t path = it->first.second;
      std::pair<int, utility::string_t> key(port, path);
      MITK_INFO << "Number of elements at key [ " << port << ", " << std::string(key.second.begin(), key.second.end())
                << "]: " << m_Observer.count(key);
      it = m_Observer.erase(it);
      MITK_INFO << "Number of elements at key [ " << port << ", " << std::string(key.second.begin(), key.second.end())
                << "]: " << m_Observer.count(key);
      //if yes
      // 1. store port in a temporary variable
      // 2. delete map entry
      // 3. check, if there is another observer under this port in observer map (with bool flag)
      //  3.1. if no, delete m_ServerMap entry for this port
    }
    else
    {
      ++it;
    }
  }
  MITK_INFO << "Handle delete client";
  //TODO implement
}

