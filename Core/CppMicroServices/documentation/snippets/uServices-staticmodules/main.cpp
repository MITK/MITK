#include <usConfig.h>

US_USE_NAMESPACE

//! [ImportStaticModuleIntoLib]
#include <usModuleImport.h>

US_IMPORT_MODULE(MyStaticModule)
US_LOAD_IMPORTED_MODULES(HostingModule, MyStaticModule)
//! [ImportStaticModuleIntoLib]

// This is just for illustration purposes in code snippets
extern "C" ModuleActivator* _us_module_activator_instance_MyStaticModule1() { return NULL; }
extern "C" ModuleActivator* _us_module_activator_instance_MyStaticModule2() { return NULL; }
extern "C" ModuleActivator* _us_init_resources_MyStaticModule2() { return NULL; }

//! [ImportStaticModuleIntoMain]
#include <usModuleImport.h>

US_IMPORT_MODULE(MyStaticModule1)
US_IMPORT_MODULE(MyStaticModule2)
US_IMPORT_MODULE_RESOURCES(MyStaticModule2)
US_LOAD_IMPORTED_MODULES_INTO_MAIN(MyStaticModule1 MyStaticModule2)
//! [ImportStaticModuleIntoMain]

int main(int /*argc*/, char* /*argv*/[])
{
  return 0;
}

//! [InitializeExecutable]
#include <usModuleInitialization.h>
US_INITIALIZE_EXECUTABLE("MyExecutable")
//! [InitializeExecutable]
