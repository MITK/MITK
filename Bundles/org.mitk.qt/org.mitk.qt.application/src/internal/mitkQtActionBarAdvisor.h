#ifndef MITKQTACTIONBARADVISOR_H_
#define MITKQTACTIONBARADVISOR_H_

#include <org.opencherry.ui/src/application/cherryActionBarAdvisor.h>

class mitkQtActionBarAdvisor : public cherry::ActionBarAdvisor
{
public:
  
  mitkQtActionBarAdvisor(cherry::IActionBarConfigurer::Pointer configurer);
  
protected:
  
  void FillMenuBar(void* menuBar);
};

#endif /*MITKQTACTIONBARADVISOR_H_*/
