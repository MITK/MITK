#ifndef mitkIRESTManager_h
#define mitkIRESTManager_h

#include <MitkCoreExports.h>
#include <mitkServiceInterface.h>
#include "cpprest/uri.h"

namespace mitk
{
  class IRESTManager
  {
  public:
    virtual ~IRESTManager();

    virtual void TestFunctionManager() = 0;
    //Functions for sendData() and receiveData() (sendRequest(), receiveRequest()?)

    enum RequestType
    {
      get,
      post,
      put
    };
    virtual void sendRequest(RequestType type) = 0;
    virtual void receiveRequest(web::uri uri) = 0;



  };
} // namespace mitk

MITK_DECLARE_SERVICE_INTERFACE(mitk::IRESTManager, "org.mitk.IRESTManager")

#endif
