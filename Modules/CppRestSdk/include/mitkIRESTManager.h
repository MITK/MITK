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

    virtual void SendRequest(RequestType type) = 0;

    virtual void ReceiveRequest(web::uri uri, IRESTObserver *observer) = 0;
    virtual web::json::value Handle(web::uri, web::json::value) = 0;
    virtual void HandleDeleteObserver(IRESTObserver *observer) = 0;
  };
} // namespace mitk

MITK_DECLARE_SERVICE_INTERFACE(mitk::IRESTManager, "org.mitk.IRESTManager")

#endif
