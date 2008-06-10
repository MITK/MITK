#include "mitkQtWorkbenchAdvisor.h"

#include <QmitkRegisterClasses.h>

#include "mitkQtWorkbenchWindowAdvisor.h"

void 
mitkQtWorkbenchAdvisor::Initialize(cherry::IWorkbenchConfigurer::Pointer configurer)
{
  QmitkRegisterClasses();
}

cherry::WorkbenchWindowAdvisor* 
mitkQtWorkbenchAdvisor::CreateWorkbenchWindowAdvisor(
        cherry::IWorkbenchWindowConfigurer::Pointer configurer)
{
  return new mitkQtWorkbenchWindowAdvisor(configurer);
}

std::string 
mitkQtWorkbenchAdvisor::GetInitialWindowPerspectiveId()
{
  return "";
}
