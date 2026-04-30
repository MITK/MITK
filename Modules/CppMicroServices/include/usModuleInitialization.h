/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef US_MODULE_NAME
#error Missing US_MODULE_NAME preprocessor define
#endif

#ifndef USMODULEINITIALIZATION_H
#define USMODULEINITIALIZATION_H

#include <usStaticInit_p.h>

#include <usModuleRegistry.h>
#include <usModuleInfo.h>
#include <usModuleUtils_p.h>

#include <cstring>


/**
 * \ingroup MicroServices
 *
 * \brief Creates initialization code for a module.
 *
 * Each module which wants to register itself with the CppMicroServices library
 * has to put a call to this macro in one of its source files. Further, the modules
 * source files must be compiled with the \c US_MODULE_NAME pre-processor definition
 * set to a module-unique identifier.
 *
 * Calling the \c US_INITIALIZE_MODULE macro will initialize the module for use with
 * the CppMicroServices library, using a default auto-load directory named after the
 * \c US_MODULE_NAME definition.
 *
 * \sa MicroServices_AutoLoading
 *
 * \remarks If you are using CMake, consider using the provided CMake macro
 * <code>usFunctionGenerateModuleInit()</code>.
 */
#define US_INITIALIZE_MODULE                                                                 \
namespace us {                                                                           \
namespace {                                                                                  \
                                                                                             \
/* Declare a file scoped ModuleInfo object */                                                \
US_GLOBAL_STATIC_WITH_ARGS(ModuleInfo, moduleInfo, (US_STR(US_MODULE_NAME)))                 \
                                                                                             \
/* This class is used to statically initialize the library within the C++ Micro services     \
   library. It looks up a library specific C-style function returning an instance            \
   of the ModuleActivator interface. */                                                      \
class US_ABI_LOCAL US_CONCAT(ModuleInitializer_, US_MODULE_NAME) {                           \
                                                                                             \
public:                                                                                      \
                                                                                             \
  US_CONCAT(ModuleInitializer_, US_MODULE_NAME)()                                            \
  {                                                                                          \
    ModuleInfo*(*moduleInfoPtr)() = moduleInfo;                                              \
    void* moduleInfoSym = nullptr;                                                              \
    std::memcpy(&moduleInfoSym, &moduleInfoPtr, sizeof(void*));                              \
    std::string location = ModuleUtils::GetLibraryPath(moduleInfoSym);                       \
    moduleInfoPtr()->location = location;                                                    \
                                                                                             \
    Register();                                                                              \
  }                                                                                          \
                                                                                             \
  static void Register()                                                                     \
  {                                                                                          \
    ModuleRegistry::Register(moduleInfo());                                                  \
  }                                                                                          \
                                                                                             \
  ~US_CONCAT(ModuleInitializer_, US_MODULE_NAME)()                                           \
  {                                                                                          \
    ModuleRegistry::UnRegister(moduleInfo());                                                \
  }                                                                                          \
                                                                                             \
};                                                                                           \
                                                                                             \
                                                                                             \
US_DEFINE_MODULE_INITIALIZER                                                                 \
}                                                                                            \
                                                                                             \
}                                                                             \
                                                                                             \
/* A helper function which is called by the US_IMPORT_MODULE macro to initialize             \
   static modules */                                                                         \
extern "C" void US_ABI_LOCAL US_CONCAT(_us_import_module_initializer_, US_MODULE_NAME)()     \
{                                                                                            \
  static us::US_CONCAT(ModuleInitializer_, US_MODULE_NAME) US_CONCAT(_InitializeModule_, US_MODULE_NAME); \
}

// Create a file-scoped static object for registering the module
// during static initialization of the shared library
#define US_DEFINE_MODULE_INITIALIZER \
static US_CONCAT(ModuleInitializer_, US_MODULE_NAME) US_CONCAT(_InitializeModule_, US_MODULE_NAME);

// Static modules don't create a file-scoped static object for initialization
// (it would be discarded during static linking anyway). The initialization code
// is triggered by the US_IMPORT_MODULE macro instead.
#if defined(US_STATIC_MODULE)
#undef US_DEFINE_MODULE_INITIALIZER
#define US_DEFINE_MODULE_INITIALIZER
#endif

#endif // USMODULEINITIALIZATION_H
