#ifndef mitkIRESTManager_h
#define mitkIRESTManager_h

#include "cpprest/uri.h"
#include "cpprest/json.h"
#include <MitkCoreExports.h>
#include <mitkServiceInterface.h>
#include <mitkIRESTObserver.h>

namespace mitk
{
  class IRESTManager
  {
  public:
    virtual ~IRESTManager();

    enum RequestType
    {
      get,
      post,
      put
    };
    virtual void sendRequest(RequestType type) = 0;
    virtual void receiveRequest(web::uri uri, IRESTObserver *observer) = 0;
    virtual web::json::value handle(web::uri, web::json::value) = 0;
    virtual void handleDeleteClient() = 0;
  };
} // namespace mitk

MITK_DECLARE_SERVICE_INTERFACE(mitk::IRESTManager, "org.mitk.IRESTManager")

#endif
