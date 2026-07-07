/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include <usModuleUtils_p.h>

#include <mitkLog.h>
#include <usModuleInfo.h>
#include <usUtils_p.h>

namespace us {

namespace {
  const bool sharedLibMode = true;
}

#ifdef __GNUC__

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <dlfcn.h>

std::string GetLibraryPath_impl(void* symbol)
{
  Dl_info info;
  if (dladdr(symbol, &info))
  {
    return info.dli_fname;
  }
  return "";
}

void* GetSymbol_impl(const ModuleInfo& moduleInfo, const char* symbol)
{
  // Clear the last error message
  dlerror();

  void* selfHandle = nullptr;
  if (!sharedLibMode || moduleInfo.name == "main")
  {
    // Get the handle of the executable
    selfHandle = dlopen(nullptr, RTLD_LAZY);
  }
  else
  {
    selfHandle = dlopen(moduleInfo.location.c_str(), RTLD_LAZY);
  }

  if (selfHandle)
  {
    void* addr = dlsym(selfHandle, symbol);
    dlclose(selfHandle);
    return addr;
  }
  return nullptr;
}

#elif _WIN32

#include <windows.h>

std::string GetLibraryPath_impl(void *symbol)
{
  HMODULE handle = 0;
  BOOL handleError = GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                       static_cast<LPCTSTR>(symbol), &handle);
  if (!handleError)
  {
    // Test
    return "";
  }

  char modulePath[512];
  if (GetModuleFileName(handle, modulePath, 512))
  {
    return modulePath;
  }

  return "";
}

void* GetSymbol_impl(const ModuleInfo& moduleInfo, const char* symbol)
{
  HMODULE handle = nullptr;
  if (!sharedLibMode || moduleInfo.name == "main")
  {
    handle = GetModuleHandle(nullptr);
  }
  else
  {
    handle = GetModuleHandle(moduleInfo.location.c_str());
  }

  if (!handle)
  {
    return 0;
  }

  void* addr = (void*)GetProcAddress(handle, symbol);
  return addr;
}
#else
std::string GetLibraryPath_impl(void*)
{
  return "";
}

void* GetSymbol_impl(const ModuleInfo&, const char* symbol)
{
  return 0;
}
#endif

std::string ModuleUtils::GetLibraryPath(void* symbol)
{
  return GetLibraryPath_impl(symbol);
}

void* ModuleUtils::GetSymbol(const ModuleInfo& module, const char* symbol)
{
  return GetSymbol_impl(module, symbol);
}

}
