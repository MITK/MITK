#ifndef MITKRESTTEST_H_
#define MITKRESTTEST_H_
#include "mitkIRESTManager.h"
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usServiceTracker.h>
#include "mitkIRESTObserver.h"
#include "cpprest/json.h"

namespace mitk
{
  class RestTest: public IRESTObserver
  {
  public:
    RestTest();
    ~RestTest();
    void TestRESTServer();
    web::json::value notify(web::json::value data) override;

  private:
  };
} // namespace mitk
#endif // !mitkRESTTest_h
