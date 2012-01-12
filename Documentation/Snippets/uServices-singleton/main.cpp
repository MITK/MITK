#include <mitkCommon.h>
#include <mitkModuleActivator.h>
#include <mitkModuleContext.h>

#include "SingletonOne.h"
#include "SingletonTwo.h"

class MyActivator : public mitk::ModuleActivator
{

public:

  //![0]
  void Load(mitk::ModuleContext* context)
  {
    // The Load() method of the module activator is called during static
    // initialization time of the shared library.

    // First create and register a SingletonTwoService instance.
    m_SingletonTwo = SingletonTwoService::New();
    context->RegisterService<SingletonTwoService>(m_SingletonTwo);

    // Now the SingletonOneService constructor will get a valid
    // SingletonTwoService instance.
    m_SingletonOne = SingletonOneService::New();
    m_SingletonOneReg = context->RegisterService<SingletonOneService>(m_SingletonOne);
  }
  //![0]
  
  //![1]
  void Unload(mitk::ModuleContext* /*context*/)
  {
    // Services are automatically unregistered during unloading of
    // the shared library after the call to Unload(mitk::ModuleContext*)
    // has returned.

    // Since SingletonOneService needs a non-null SingletonTwoService
    // instance in its destructor, we explicitly unregister and delete the
    // SingletonOneService instance here. This way, the SingletonOneService
    // destructor will still get a valid SingletonTwoService instance.
    m_SingletonOneReg.Unregister();
    m_SingletonOne = 0;

    // For singletonTwoService, we rely on the automatic unregistering
    // by the service registry and on automatic deletion due to the
    // smart pointer reference counting. You must not delete service instances
    // in this method without unregistering them first.
  }
  //![1]

private:

  // We use smart pointers for automatic garbage collection
  SingletonOneService::Pointer m_SingletonOne;
  SingletonTwoService::Pointer m_SingletonTwo;

  mitk::ServiceRegistration m_SingletonOneReg;

};

MITK_EXPORT_MODULE_ACTIVATOR(uServices_singleton, MyActivator)

int main()
{
}
