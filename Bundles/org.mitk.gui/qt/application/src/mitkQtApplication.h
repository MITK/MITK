#ifndef MITKQTAPPLICATION_H_
#define MITKQTAPPLICATION_H_

#include <application/cherryIApplication.h>

class mitkQtApplication : public cherry::IApplication
{
public:
  
  int Start();
  void Stop();
};

#endif /*MITKQTAPPLICATION_H_*/
