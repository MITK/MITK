#ifndef MITKQTWORKBENCHWINDOWADVISOR_H_
#define MITKQTWORKBENCHWINDOWADVISOR_H_

#include <application/cherryWorkbenchWindowAdvisor.h>

class mitkQtWorkbenchWindowAdvisor : public cherry::WorkbenchWindowAdvisor
{
public:
  
  mitkQtWorkbenchWindowAdvisor(cherry::IWorkbenchWindowConfigurer::Pointer configurer);
  
  cherry::ActionBarAdvisor::Pointer CreateActionBarAdvisor(
      cherry::IActionBarConfigurer::Pointer configurer);
};

#endif /*MITKQTWORKBENCHWINDOWADVISOR_H_*/
