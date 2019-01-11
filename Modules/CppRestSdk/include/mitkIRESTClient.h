#ifndef mitkIRESTClient_h
#define mitkIRESTClient_h

#include <MitkCoreExports.h>
#include <mitkServiceInterface.h>

namespace mitk
{
  class IRESTClient
  {
  public:
    virtual ~IRESTClient();

    virtual void TestFunctionClient() = 0;
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::IRESTClient, "org.mitk.IRESTClient")
#endif


