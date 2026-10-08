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

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace us {

/**
 * \brief Size of the range between two linker-provided addresses.
 *
 * The volatile copies keep the compiler from assuming that distinct symbols
 * have distinct addresses, which does not hold for linker-provided section
 * boundaries: the start and end of an empty section coincide.
 */
inline std::size_t ModuleResourceRangeSize(const char* begin, const char* end)
{
  const char* volatile first = begin;
  const char* volatile last = end;
  return static_cast<std::size_t>(reinterpret_cast<std::uintptr_t>(last) - reinterpret_cast<std::uintptr_t>(first));
}

}

// The module hands a linked resource archive to the resource container as a
// memory range, and tells it whether its file may carry appended resources;
// only such files are searched.
//
// ELF: usFunctionEmbedResources puts a linked archive into the section
// us_resources, whose bounds the linker provides for each binary as
// __start_us_resources and __stop_us_resources, and gives a module with
// appended resources the marker us_resources_appended_<name>. The weak
// references resolve to nullptr where these are missing. They are hidden, so
// they bind only within the binary being linked: linkers may export the
// section bounds, and a module without resources would otherwise resolve
// another module's archive.
//
// Mach-O: the linker resolves section$start and section$end within the image
// being linked, and to an empty range if the section does not exist, so the
// archive section (__TEXT,us_resources) and the marker section
// (__DATA,us_appended) need no renaming.
//
// Windows: the archive is a resource of the binary, found through the address
// of a symbol in it.
#if defined(__ELF__)
#define US_DECLARE_MODULE_RESOURCES                                                          \
extern "C" const char __start_us_resources[] __attribute__((weak, visibility("hidden")));    \
extern "C" const char __stop_us_resources[] __attribute__((weak, visibility("hidden")));     \
extern "C" const char US_CONCAT(us_resources_appended_, US_MODULE_NAME) __attribute__((weak));

#define US_SET_MODULE_RESOURCES(info, symbol)                                                \
  if (__start_us_resources != nullptr)                                                       \
  {                                                                                          \
    info->resourceData = __start_us_resources;                                               \
    info->resourceSize = us::ModuleResourceRangeSize(__start_us_resources, __stop_us_resources); \
  }                                                                                          \
  info->resourcesInFile = &US_CONCAT(us_resources_appended_, US_MODULE_NAME) != nullptr;
#elif defined(__APPLE__)
#define US_DECLARE_MODULE_RESOURCES                                                          \
extern "C" const char US_CONCAT(us_resources_start_, US_MODULE_NAME) __asm("section$start$__TEXT$us_resources"); \
extern "C" const char US_CONCAT(us_resources_end_, US_MODULE_NAME) __asm("section$end$__TEXT$us_resources");     \
extern "C" const char US_CONCAT(us_appended_start_, US_MODULE_NAME) __asm("section$start$__DATA$us_appended");   \
extern "C" const char US_CONCAT(us_appended_end_, US_MODULE_NAME) __asm("section$end$__DATA$us_appended");

#define US_SET_MODULE_RESOURCES(info, symbol)                                                \
  info->resourceSize = us::ModuleResourceRangeSize(                                          \
    &US_CONCAT(us_resources_start_, US_MODULE_NAME), &US_CONCAT(us_resources_end_, US_MODULE_NAME)); \
  if (info->resourceSize != 0)                                                               \
  {                                                                                          \
    info->resourceData = &US_CONCAT(us_resources_start_, US_MODULE_NAME);                    \
  }                                                                                          \
  info->resourcesInFile = us::ModuleResourceRangeSize(                                       \
    &US_CONCAT(us_appended_start_, US_MODULE_NAME), &US_CONCAT(us_appended_end_, US_MODULE_NAME)) != 0;
#elif defined(_WIN32)
#define US_DECLARE_MODULE_RESOURCES
#define US_SET_MODULE_RESOURCES(info, symbol) us::ModuleUtils::GetLinkedResources(symbol, info);
#else
#define US_DECLARE_MODULE_RESOURCES
#define US_SET_MODULE_RESOURCES(info, symbol)
#endif


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
US_DECLARE_MODULE_RESOURCES                                                                  \
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
    US_SET_MODULE_RESOURCES(moduleInfoPtr(), moduleInfoSym)                                  \
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
/* Registers the module during static initialization of the shared library. */               \
static US_CONCAT(ModuleInitializer_, US_MODULE_NAME) US_CONCAT(_InitializeModule_, US_MODULE_NAME); \
}                                                                                            \
                                                                                             \
}

#endif // USMODULEINITIALIZATION_H
