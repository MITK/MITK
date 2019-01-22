#ifndef mitkIRESTManager_h
#define mitkIRESTManager_h

#include <MitkCoreExports.h>
#include <mitkServiceInterface.h>

namespace mitk
{
  class IRESTManager
  {
  public:
    virtual ~IRESTManager();

    virtual void TestFunctionManager() = 0;
  };
} // namespace mitk

#endif
