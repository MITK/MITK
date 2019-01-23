#ifndef mitkRESTServerMicroService_h
#define mitkRESTServerMicroService_h

#include "cpprest/asyncrt_utils.h"
#include "cpprest/containerstream.h"
#include "cpprest/filestream.h"
#include "cpprest/http_listener.h"
#include "cpprest/json.h"
#include "cpprest/producerconsumerstream.h"
#include "cpprest/uri.h"

#include "MitkCppRestSdkExports.h"

typedef web::http::experimental::listener::http_listener MitkListener;
typedef web::http::http_request MitkRequest;
typedef web::http::http_response MitkResponse;
typedef web::http::methods MitkRESTMethods;
typedef web::http::status_codes MitkRestStatusCodes;
typedef web::json::json_exception MitkJsonException;


namespace mitk
{
  class RESTServerMicroService
  {
  public:
    RESTServerMicroService(web::uri uri);
    ~RESTServerMicroService();
    pplx::task<void> mitk::RESTServerMicroService::openListener();
    void HandleGet(MitkRequest request);

  private:
    MitkListener m_Listener;
  };
} // namespace mitk
#endif 