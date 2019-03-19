#include "mitkRESTManagerQt.h"
#include <QCoreApplication>
#include <mitkCommon.h>
mitk::RESTManagerQt::RESTManagerQt() {}

mitk::RESTManagerQt::~RESTManagerQt() {}


pplx::task<web::json::value> mitk::RESTManagerQt::SendRequest(const web::uri &uri,
                                                            const RequestType &type,
                                                            const web::json::value &content,
                                                            const utility::string_t &filePath)
{
  return mitk::RESTManager::SendRequest(uri, type, content, filePath);
}


void mitk::RESTManagerQt::ReceiveRequest(const web::uri &uri, mitk::IRESTObserver *observer)
{
  // New instance of RESTServerMicroservice in m_ServerMap, key is port of the request
  int port = uri.port();

  // Checking if port is free to add a new Server
  if (m_ServerMap.count(port) == 0)
  {
    mitk::RESTManager::AddObserver(uri, observer);

    // creating server instance
    RESTServerMicroServiceQt *server = new RESTServerMicroServiceQt(uri.authority());
    // add reference to server instance to map
    m_ServerMap[port] = server;

    // Move server to seperate Thread and create connections between threads
    m_ServerThreadMap[port] = new QThread;
    server->moveToThread(m_ServerThreadMap[port]);

    connect(m_ServerThreadMap[port], &QThread::finished, server, &QObject::deleteLater);

    // starting Server
    m_ServerThreadMap[port]->start();
    QMetaObject::invokeMethod(server, "OpenListener");

    utility::string_t host = uri.authority().to_string();
    std::string hoststring(host.begin(), host.end());
    MITK_INFO << "new server" << hoststring << " at port" << port;
  }
  // If there is already a server under this port
  else
  {
    mitk::RESTManager::ServerUnderPort(uri, observer);
  }
}

 web::json::value mitk::RESTManagerQt::Handle(const web::uri &uri, web::json::value &body)
{
   return mitk::RESTManager::Handle(uri, body);
}

void mitk::RESTManagerQt::HandleDeleteObserver(IRESTObserver *observer, const web::uri &uri = L"")
{
  for (auto it = m_Observers.begin(); it != m_Observers.end();)
  {
    mitk::IRESTObserver *obsMap = it->second;
    // Check wether observer is at this place in map
    if (obsMap == observer)
    {
      // Check wether it is the right uri to be deleted
      if (uri == L"" || it->first.second == uri.path())
      {
        int port = it->first.first;
        bool noObserverForPort = mitk::RESTManager::DeleteObserver(it, uri);
        if (noObserverForPort)
        {
          //  there isn't an observer at this port, delete m_ServerMap entry for this port
          // close listener
          QMetaObject::invokeMethod(dynamic_cast<mitk::RESTServerMicroServiceQt *>(m_ServerMap[port]), "CloseListener");
          // end thread
          m_ServerThreadMap[port]->quit();
          m_ServerThreadMap[port]->wait();
 
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

std::map<int, mitk::IRESTServerMicroService *> mitk::RESTManagerQt::GetM_ServerMap()
{
  return mitk::RESTManager::GetM_ServerMap();
}

std::map<std::pair<int, utility::string_t>, mitk::IRESTObserver *> mitk::RESTManagerQt::GetM_Observers()
{
  return mitk::RESTManager::GetM_Observers();
}