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

    void TestFunctionManager() override;

  private:
    std::map<int, RESTClientMicroService> clientMap; //Map with port client pairs
    std::map<int, RESTServerMicroService> serverMap; //Map with port server pairs
  };
} // namespace mitk
#endif // !mitkRESTManager_h
