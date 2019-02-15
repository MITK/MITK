#ifndef mitkRESTClientMicroService_h
#define mitkRESTClientMicroService_h

#include "cpprest/asyncrt_utils.h"
#include "cpprest/containerstream.h"
//#include "cpprest/filestream.h"
#include "cpprest/http_client.h"
#include "cpprest/json.h"
#include "cpprest/producerconsumerstream.h"
#include "cpprest/uri.h"

#include "MitkCppRestSdkExports.h"
#include <mitkIRESTManager.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usServiceTracker.h>

typedef web::http::client::http_client MitkClient;
typedef web::http::http_request MitkRequest;
typedef web::http::http_response MitkResponse;
typedef web::http::methods MitkRESTMethods;
typedef web::http::uri_builder MitkUriBuilder;
typedef web::http::status_codes MitkRestStatusCodes;
typedef web::json::json_exception MitkJsonException;

namespace mitk
{
  class RESTClientMicroService
  {
  public:
    RESTClientMicroService();
    ~RESTClientMicroService();

   pplx::task<web::json::value> Get(web::uri);
   pplx::task<web::json::value> PUT(web::uri, web::json::value content);
   pplx::task<web::json::value> POST(web::uri, web::json::value content);
  };
} // namespace mitk
#endif // !mitkRESTClientMicroService_h
