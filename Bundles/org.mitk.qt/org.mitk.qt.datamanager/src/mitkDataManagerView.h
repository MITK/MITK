#ifndef MITKDATAMANAGERVIEW_H_
#define MITKDATAMANAGERVIEW_H_

#include <org.opencherry.ui/src/cherryViewPart.h>

class mitkDataManagerView : public cherry::ViewPart
{
  
public:
  void* CreatePartControl(void* parent);
  void SetFocus();
 
};

#endif /*MITKDATAMANAGERVIEW_H_*/
