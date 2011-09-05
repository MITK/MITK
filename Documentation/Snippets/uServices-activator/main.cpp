//! [0]
#include <mitkCommon.h>
#include <mitkModuleActivator.h>

class MyActivator : public mitk::ModuleActivator
{

public:

  void Load(mitk::ModuleContext* context)
  { /* register stuff */ }
  

  void Unload(mitk::ModuleContext* context)
  { /* cleanup */ }

};

MITK_EXPORT_MODULE_ACTIVATOR(mylibname, MyActivator)
//![0]

int main(int argc, char* argv[])
{
  MyActivator ma;
  return 0;
}
