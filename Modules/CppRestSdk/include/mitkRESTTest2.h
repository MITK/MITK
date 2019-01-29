#ifndef MITKRESTTEST2_H_
#define MITKRESTTEST2_H_
#include "cpprest/json.h"
#include "mitkIRESTManager.h"
#include "mitkIRESTObserver.h"
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usServiceTracker.h>

namespace mitk
{
  class RestTest2 : public IRESTObserver
  {
  public:
    RestTest2();
    ~RestTest2();
    void TestRESTServer();
    web::json::value notify(web::json::value data) override;

  private:
  };
} // namespace mitk
#endif // !mitkRESTTest_h