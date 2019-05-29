/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <mitkRESTManager.h>
#include <mitkRESTClient.h>
#include <mitkRESTServer.h>
#include <mitkIRESTObserver.h>

#include <mitkExceptionMacro.h>
#include <mitkLogMacros.h>

mitk::RESTManager::RESTManager()
{
}

mitk::RESTManager::~RESTManager()
{
}

pplx::task<web::json::value> mitk::RESTManager::SendRequest(const web::uri &uri,
                                                            const RequestType &type,
                                                            const web::json::value *content,
                                                            const utility::string_t &filePath)
{
  pplx::task<web::json::value> answer;
  auto client = new RESTClient;

  switch (type)
  {
    case RequestType::Get:
      answer = !filePath.empty()
        ? client->Get(uri, filePath)
        : client->Get(uri);
      break;

    case RequestType::Post:
      if (nullptr == content)
        MITK_WARN << "Content for put is empty, this will create an empty resource";

      answer = client->Post(uri, content);
      break;

    case RequestType::Put:

      if (nullptr == content)
        MITK_WARN << "Content for put is empty, this will empty the ressource";

      answer = client->Put(uri, content);
      break;

    default:
      mitkThrow() << "Request Type not supported";
  }

  return answer;
}

void mitk::RESTManager::ReceiveRequest(const web::uri &uri, mitk::IRESTObserver *observer)
{
  // New instance of RESTServer in m_ServerMap, key is port of the request
  auto port = uri.port();

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
  auto key = std::make_pair(uri.port(), uri.path());
  if (0 != m_Observers.count(key))
  {
    return m_Observers[key]->Notify(uri, body);
  }
  // No observer under this port, return null which results in status code 404 (s. RESTServer)
  else
  {
    MITK_WARN << "No Observer can handle the data";
    return web::json::value();
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
        bool noObserverForPort = this->DeleteObserver(it);
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

bool mitk::RESTManager::DeleteObserver(std::map<std::pair<int, utility::string_t>, IRESTObserver *>::iterator &it)
{
  int port = it->first.first;

  it = m_Observers.erase(it);

  for (auto observer : m_Observers)
  {
    if (port == observer.first.first)
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
