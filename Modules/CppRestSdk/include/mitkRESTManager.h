#ifndef mitkRESTManager_h
#define mitkRESTManager_h

#include <QObject>
#include <QThread>

#include <mitkIRESTManager.h>
#include <mitkIRESTObserver.h>
#include <mitkRESTClientMicroService.h>
#include <mitkRESTServerMicroService.h>

#include <MitkCppRestSdkExports.h>

namespace mitk
{
  class MITKCPPRESTSDK_EXPORT RESTManager : public QObject, public IRESTManager
  {
     Q_OBJECT

  public:
    RESTManager();
    ~RESTManager() override;

    // calls RESTClient
    pplx::task<web::json::value> SendRequest(web::uri uri, RequestType type = get, web::json::value = NULL) override;
    // calls RESTServer
    void ReceiveRequest(web::uri uri, IRESTObserver *observer) override;

    web::json::value Handle(web::uri, web::json::value) override;

    //virtual void HandleDeleteObserver(IRESTObserver *observer) override;
    virtual void HandleDeleteObserver(IRESTObserver *observer, web::uri) override;

  private:
    std::map<int, RESTClientMicroService *> m_ClientMap;                     // Map with port client pairs
    std::map<int, RESTServerMicroService *> m_ServerMap;                     // Map with port server pairs
    std::map<int, QThread*> m_ServerThreadMap;                               // Map with threads for servers
    std::map<std::pair<int, utility::string_t>, IRESTObserver *> m_Observer; // Map with all observers
  };
} // namespace mitk
#endif // !mitkRESTManager_h
