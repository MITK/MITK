#ifndef MITKQTWORKBENCHADVISOR_H_
#define MITKQTWORKBENCHADVISOR_H_

#include <org.opencherry.ui/src/application/cherryWorkbenchAdvisor.h>

class mitkQtWorkbenchAdvisor : public cherry::WorkbenchAdvisor
{
public:
  
  void Initialize(cherry::IWorkbenchConfigurer::Pointer configurer);
  
  cherry::WorkbenchWindowAdvisor* CreateWorkbenchWindowAdvisor(
        cherry::IWorkbenchWindowConfigurer::Pointer configurer);
  
  std::string GetInitialWindowPerspectiveId();
};

#endif /*MITKQTWORKBENCHADVISOR_H_*/
