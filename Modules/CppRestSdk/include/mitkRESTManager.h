#ifndef mitkRESTManager_h
#define mitkRESTManager_h

#include <mitkIRESTManager.h>
#include <mitkRESTClientMicroService.h>
#include <mitkRESTServerMicroService.h>
#include <mitkIRESTObserver.h>

namespace mitk
{
  class RESTManager : public IRESTManager
  {
  public:
    RESTManager();
    ~RESTManager() override;

    //calls RESTClient
    void sendRequest(RequestType type) override;
    //calls RESTServer
    void receiveRequest(web::uri uri, IRESTObserver *observer) override;

    bool handle(web::uri) override;

  private:
    std::map<int, RESTClientMicroService*> m_ClientMap; // Map with port client pairs
    std::map<int, RESTServerMicroService*> m_ServerMap; // Map with port server pairs
    std::map<web::uri, IRESTObserver *> m_Observer; //Map with all observers
  };
} // namespace mitk
#endif // !mitkRESTManager_h
