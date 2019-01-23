#ifndef mitkRESTManager_h
#define mitkRESTManager_h

#include <mitkIRESTManager.h>
#include <mitkRESTClientMicroService.h>
#include <mitkRESTServerMicroService.h>

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
    void receiveRequest(web::uri uri) override;

  private:
    std::map<int, RESTClientMicroService*> m_ClientMap; // Map with port client pairs
    std::map<int, RESTServerMicroService*> m_ServerMap; // Map with port server pairs
  };
} // namespace mitk
#endif // !mitkRESTManager_h
