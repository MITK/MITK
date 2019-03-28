#include "mitkRESTManager.h"
#include <mitkCommon.h>
#include <mitkRESTUtil.h>

mitk::RESTManager::RESTManager() {}

mitk::RESTManager::~RESTManager() {}

pplx::task<web::json::value> mitk::RESTManager::SendRequest(const web::uri &uri,
                                                            const RequestType &type,
                                                            const web::json::value *content,
                                                            const utility::string_t &filePath)
{
  pplx::task<web::json::value> answer;
  auto client = new RESTClient();
  // according to the RequestType, different HTTP requests are made
  switch (type)
  {
    case RequestType::Get:
    
      if (filePath.empty())
      {
        // no file path specified, starts a normal get request returning the normal json result
        answer = client->Get(uri);
      }
      else
      {
        // file path ist specified, the result of the get request ist stored in this file
        // and an empty json object is returned
        answer = client->Get(uri, filePath);
      }
      break;
    
    case RequestType::Post:
    
      if (nullptr == content)
      {
        // warning because normally you won't create an empty ressource
        MITK_WARN << "Content for put is empty, this will create an empty ressource";
      }
      answer = client->Post(uri, content);
      break;
    
    case RequestType::Put:
    
      if (nullptr == content)
      {
        // warning because normally you won't empty a ressource
        MITK_WARN << "Content for put is empty, this will empty the ressource";
      }
      answer = client->Put(uri, content);
      break;
    
    default:
      mitkThrow() << "Request Type not supported";
      break;
  }

  return answer;
}

void mitk::RESTManager::ReceiveRequest(const web::uri &uri, mitk::IRESTObserver *observer)
{
  // New instance of RESTServer in m_ServerMap, key is port of the request
  int port = uri.port();

  // Checking if port is free to add a new Server
  if (0 == m_ServerMap.count(port))
  {
    this->AddObserver(uri, observer);
    // creating server instance
    auto server = new RESTServer(uri.authority());
    // add reference to server instance to map
    m_ServerMap[port] = server;
    // start Server
    server->OpenListener();

    MITK_INFO << "new server " << mitk::RESTUtil::convertToUtf8(uri.authority().to_string()) << " at port " << port;
  }
  // If there is already a server under this port
  else
  {
    this->RequestForATakenPort(uri, observer);
  }
}

web::json::value mitk::RESTManager::Handle(const web::uri &uri, const web::json::value &body)
{
  // Checking if there is an observer for the port and path
  std::pair<int, utility::string_t> key(uri.port(), uri.path());
  if (0 != m_Observers.count(key))
  {
    return m_Observers[key]->Notify(uri, body);
  }
  // No observer under this port, return null which results in status code 404 (s. RESTServer)
  else
  {
    MITK_WARN << "No Observer can handle the data";
    return NULL;
  }
}

void mitk::RESTManager::HandleDeleteObserver(IRESTObserver *observer, const web::uri &uri)
{
  for (auto it = m_Observers.begin(); it != m_Observers.end();)
  {
    mitk::IRESTObserver *obsMap = it->second;
    // Check wether observer is at this place in map
    if (observer == obsMap)
    {
      // Check wether it is the right uri to be deleted
      if (uri.is_empty() || uri.path() == it->first.second)
      {
        int port = it->first.first;
        bool noObserverForPort = this->DeleteObserver(it, uri);
        if (noObserverForPort)
        {
          //  there isn't an observer at this port, delete m_ServerMap entry for this port
          // close listener
          m_ServerMap[port]->CloseListener();
          delete m_ServerMap[port];
          // delete server from map
          m_ServerMap.erase(port);
        }
      }
      else
      {
        ++it;
      }
    }
    else
    {
      ++it;
    }
  }
}

const std::map<int, mitk::RESTServer *> &mitk::RESTManager::GetServerMap()
{
  return m_ServerMap;
}

std::map<std::pair<int, utility::string_t>, mitk::IRESTObserver *> &mitk::RESTManager::GetObservers()
{
  return m_Observers;
}

void mitk::RESTManager::AddObserver(const web::uri &uri, IRESTObserver *observer)
{
  // new observer has to be added
  std::pair<int, utility::string_t> key(uri.port(), uri.path());
  m_Observers[key] = observer;
}

void mitk::RESTManager::RequestForATakenPort(const web::uri &uri, IRESTObserver *observer)
{
  // Same host, means new observer but not a new server instance
  if (uri.authority() == m_ServerMap[uri.port()]->GetUri())
  {
    // new observer has to be added
    std::pair<int, utility::string_t> key(uri.port(), uri.path());
    // only add a new observer if there isn't already an observer for this uri
    if (0 == m_Observers.count(key))
    {
      this->AddObserver(uri, observer);

      // info output
      MITK_INFO << "started listening, no new server instance has been created";
    }
    else
    {
      MITK_ERROR << "Threre is already a observer handeling this data";
    }
  }
  // Error, since another server can't be added under this port
  else
  {
    MITK_ERROR << "There is already another server listening under this port";
  }
}

bool mitk::RESTManager::DeleteObserver(std::map<std::pair<int, utility::string_t>, IRESTObserver *>::iterator &it,
                                       const web::uri &uri)
{
  // if yes
  // 1. store port and path in a temporary variable
  // (path is only needed to create a key for info output)
  int port = it->first.first;
  utility::string_t path = it->first.second;
  std::pair<int, utility::string_t> key(port, path);
  // 2. delete map entry
  it = m_Observers.erase(it);
  // 3. check, if there is another observer under this port in observer map (with bool flag)
  for (auto o : m_Observers)
  {
    if (port == o.first.first)
    {
      // there still exists an observer for this port
      return false;
    }
  }
  return true;
}

void mitk::RESTManager::SetServerMap(const int port, RESTServer *server) 
{
  m_ServerMap[port] = server;
}

void mitk::RESTManager::DeleteFromServerMap(const int port) 
{
  m_ServerMap.erase(port);
}

void mitk::RESTManager::SetObservers(const std::pair<int, utility::string_t> key, IRESTObserver *observer) 
{
  m_Observers[key] = observer;
}
