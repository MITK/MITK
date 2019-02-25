#include "mitkRESTManager.h"
#include <mitkCommon.h>

mitk::RESTManager::RESTManager() {}

mitk::RESTManager::~RESTManager() {}

pplx::task<web::json::value> mitk::RESTManager::SendRequest(web::uri uri,
                                                            RequestType type,
                                                            web::json::value content,
                                                            utility::string_t filePath)
{
  pplx::task<web::json::value> answer;
  auto client = new RESTClientMicroService();
  switch (type)
  {
    case get:
    {
      if (filePath == L"")
      {
        answer = client->Get(uri);
      }
      else
      {
        answer = client->Get(uri, filePath);
      }
      break;
    }
    case post:
    {
      if (content == NULL)
      {
        MITK_WARN << "Content for put is empty, this will create an empty ressource";
      }
      answer = client->POST(uri, content);
      break;
    }
      break;
    case put:
    {
      if (content == NULL)
      {
        MITK_WARN << "Content for put is empty, this will empty the ressource";
      }
      answer = client->PUT(uri,content);
      break;
    }
  }

  return answer;
}

void mitk::RESTManager::ReceiveRequest(web::uri uri, mitk::IRESTObserver *observer)
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
    MITK_INFO <<"[" <<uri.port()<<", " << uristring << "] : Number of elements in map: " << m_Observer.count(key);

    // creating server instance
    RESTServerMicroService *server = new RESTServerMicroService(uri.authority());
    // add reference to server instance to map
    m_ServerMap[port] = server;

    //Move server to seperate Thread and create connections between threads
    m_ServerThreadMap[port] = new QThread();
    server->moveToThread(m_ServerThreadMap[port]);

    connect(m_ServerThreadMap[port], &QThread::finished, server, &QObject::deleteLater);

    //starting Server
    m_ServerThreadMap[port]->start();
    QMetaObject::invokeMethod(server, "OpenListener");
 
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
      //only add a new observer if there isn't already an observer for this uri
      if (m_Observer.count(key) == 0)
      {
        m_Observer[key] = observer;

        // testing if entry has been added to map
        utility::string_t uristringt = uri.path();
        std::string uristring(uristringt.begin(), uristringt.end());
        MITK_INFO << "[" << uri.port() << ", " << uristring<< "] : Number of elements in map: " << m_Observer.count(key);

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
}


web::json::value mitk::RESTManager::Handle(web::uri uri, web::json::value data)
{
  // Checking if is there is a observer for the port and path
  std::pair<int, utility::string_t> key(uri.port(), uri.path());
  if (m_Observer.count(key) != 0)
  {
    MITK_INFO << "Manager: Data send to observer";
    return m_Observer[key]->Notify(data);
  }
  //No map under this port, delete Server Object to release port for other servers
  else
  {
    MITK_WARN << "No Observer can handle the data";
    return NULL;
  }
}

//void mitk::RESTManager::HandleDeleteObserver(mitk::IRESTObserver *observer)
//{
//  for (auto it = m_Observer.begin(); it != m_Observer.end();)
//  {
//    mitk::IRESTObserver *obsMap = it->second;
//    //Check weather observer is at this place in map
//    if (obsMap == observer)
//    {
//      // if yes
//      // 1. store port in a temporary variable
//      int port = it->first.first;
//      utility::string_t path = it->first.second;
//      std::pair<int, utility::string_t> key(port, path);
//      MITK_INFO << "Number of elements at key [ " << port << ", " << std::string(key.second.begin(), key.second.end())
//                << "]: " << m_Observer.count(key);
//      // 2. delete map entry
//      it = m_Observer.erase(it);
//      MITK_INFO << "Number of elements at key [ " << port << ", " << std::string(key.second.begin(), key.second.end())
//                << "]: " << m_Observer.count(key);
//      // 3. check, if there is another observer under this port in observer map (with bool flag)
//      bool noObserverForPort = true;
//      for (auto o : m_Observer)
//      {
//        if (o.first.first == port)
//        {
//          //there still exists an observer for this port
//          noObserverForPort = false;
//        }
//      }
//      if (noObserverForPort)
//      {
//        //  there isn't an observer at this port, delete m_ServerMap entry for this port  
//        //close listener
//        QMetaObject::invokeMethod(m_ServerMap[port], "CloseListener");
//        //end thread
//        m_ServerThreadMap[port]->quit();
//        m_ServerThreadMap[port]->wait();
//        //delete server from map
//        m_ServerMap.erase(port);
//      }     
//    }
//    else
//    {
//      ++it;
//    }
//  }
//}

void mitk::RESTManager::HandleDeleteObserver(IRESTObserver *observer, web::uri uri=L"")
{
  for (auto it = m_Observer.begin(); it != m_Observer.end();)
  {
    mitk::IRESTObserver *obsMap = it->second;
    // Check wether observer is at this place in map
    if (obsMap == observer)
    {
      //Check wether it is the right uri to be deleted
      if (uri==L""||it->first.second == uri.path())
      {
        // if yes
        // 1. store port in a temporary variable
        int port = it->first.first;
        utility::string_t path = it->first.second;
        std::pair<int, utility::string_t> key(port, path);
        MITK_INFO << "Number of elements at key [ " << port << ", " << std::string(key.second.begin(), key.second.end())
                  << "]: " << m_Observer.count(key);
        // 2. delete map entry
        it = m_Observer.erase(it);
        MITK_INFO << "Number of elements at key [ " << port << ", " << std::string(key.second.begin(), key.second.end())
                  << "]: " << m_Observer.count(key);
        // 3. check, if there is another observer under this port in observer map (with bool flag)
        bool noObserverForPort = true;
        for (auto o : m_Observer)
        {
          if (o.first.first == port)
          {
            // there still exists an observer for this port
            noObserverForPort = false;
          }
        }
        if (noObserverForPort)
        {
          //  there isn't an observer at this port, delete m_ServerMap entry for this port
          // close listener
          QMetaObject::invokeMethod(m_ServerMap[port], "CloseListener");
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

