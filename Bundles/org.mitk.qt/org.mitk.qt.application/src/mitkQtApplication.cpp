#include "mitkQtApplication.h"

#include <org.opencherry.ui/src/cherryPlatformUI.h>

#include "internal/mitkQtWorkbenchAdvisor.h"

int mitkQtApplication::Start()
{
  int code = cherry::PlatformUI::CreateAndRunWorkbench(new mitkQtWorkbenchAdvisor());
  
  // exit the application with an appropriate return code
  return code == cherry::PlatformUI::RETURN_RESTART
              ? EXIT_RESTART : EXIT_OK;
}

void mitkQtApplication::Stop()
{
  
}
