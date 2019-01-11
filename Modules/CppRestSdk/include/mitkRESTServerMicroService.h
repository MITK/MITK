#ifndef mitkRESTServerMicroService_h
#define mitkRESTServerMicroService_h

#include <mitkIRESTServer.h>

namespace mitk
{
  class RESTServerMicroService : public IRESTServer
  {
  public:
    RESTServerMicroService();
    ~RESTServerMicroService() override;

    void TestFunctionServer() override;
  };
} // namespace mitk
#endif 