#ifndef mitkIRESTObserver_h
#define mitkIRESTObserver_h

#include "cpprest/json.h"
#include <MitkCppRestSdkExports.h>

namespace mitk
{
  class MITKCPPRESTSDK_EXPORT IRESTObserver
  {
  public:
    virtual ~IRESTObserver();
    virtual web::json::value Notify(web::json::value data) = 0;


  private:
  };
}

#endif // !mitkIRESTObserver
