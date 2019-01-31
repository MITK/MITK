#ifndef mitkIRESTObserver_h
#define mitkIRESTObserver_h

#include "cpprest/json.h"

namespace mitk
{
  class IRESTObserver
  {
  public:
    virtual ~IRESTObserver();
    virtual web::json::value notify(web::json::value data) = 0;


  private:
  };
}

#endif // !mitkIRESTObserver
