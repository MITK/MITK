#include "mitkQtWorkbenchWindowAdvisor.h"

#include "mitkQtActionBarAdvisor.h"

mitkQtWorkbenchWindowAdvisor::mitkQtWorkbenchWindowAdvisor(cherry::IWorkbenchWindowConfigurer::Pointer configurer)
 : cherry::WorkbenchWindowAdvisor(configurer)
{
  
}

cherry::ActionBarAdvisor::Pointer 
mitkQtWorkbenchWindowAdvisor::CreateActionBarAdvisor(
    cherry::IActionBarConfigurer::Pointer configurer)
{
  cherry::ActionBarAdvisor::Pointer actionBarAdvisor = new mitkQtActionBarAdvisor(configurer);
  return actionBarAdvisor;
}
