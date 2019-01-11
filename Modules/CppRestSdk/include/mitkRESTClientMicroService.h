#ifndef mitkRESTClientMicroService_h
#define mitkRESTClientMicroService_h

#include <mitkIRESTClient.h>

namespace mitk
{
  class RESTClientMicroService : public IRESTClient
  {
  public:
    RESTClientMicroService();
    ~RESTClientMicroService() override;

    void TestFunctionClient() override;
  };
} // namespace mitk
#endif // !mitkRESTClientMicroService_h
