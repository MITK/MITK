#ifndef mitkIRESTServer_h
#define mitkIRESTServer_h

#include <MitkCoreExports.h>
#include <mitkServiceInterface.h>

namespace mitk
{
  class IRESTServer
  {
  public:
    virtual ~IRESTServer();

    virtual void TestFunctionServer() = 0;
  };
} // namespace mitk

MITK_DECLARE_SERVICE_INTERFACE(mitk::IRESTServer, "org.mitk.IRESTServer")
#endif
