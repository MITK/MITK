#ifndef mitkRESTClientMicroService_h
#define mitkRESTClientMicroService_h

#include "cpprest/http_client.h"


typedef web::http::client::http_client MitkClient;

namespace mitk
{
  class RESTClientMicroService
  {
  public:
    RESTClientMicroService(utility::string_t url);
    ~RESTClientMicroService();

    void TestFunctionClient();

  };
} // namespace mitk
#endif // !mitkRESTClientMicroService_h
