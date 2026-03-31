namespace us {
struct ModuleActivator;
}

using namespace us;

// This is just for illustration purposes in code snippets
extern "C" ModuleActivator* _us_module_activator_instance_MyStaticModule1() { return nullptr; }
extern "C" void _us_import_module_initializer_MyStaticModule1() {}

//! [ImportStaticModuleIntoMain]
#include <usModuleImport.h>

US_IMPORT_MODULE(MyStaticModule1)
//! [ImportStaticModuleIntoMain]

int main(int /*argc*/, char* /*argv*/[])
{
  return 0;
}

//! [InitializeExecutable]
#include <usModuleInitialization.h>
US_INITIALIZE_MODULE
//! [InitializeExecutable]
