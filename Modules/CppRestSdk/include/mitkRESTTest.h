#ifndef MITKRESTTEST_H_
#define MITKRESTTEST_H_
#include "mitkIRESTManager.h"
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usServiceTracker.h>
#include "mitkIRESTObserver.h"

namespace mitk
{
  class RestTest: public IRESTObserver
  {
  public:
    RestTest();
    ~RestTest();
    void TestRESTServer();
    bool notify() override;

  private:
  };
} // namespace mitk
#endif // !mitkRESTTest_h
