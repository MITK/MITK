/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/


#include "usModuleUtils_p.h"

#include <usLog_p.h>
#include <usModuleInfo.h>
#include <usUtils_p.h>

US_BEGIN_NAMESPACE

namespace {
#ifdef US_BUILD_SHARED_LIBS
  const bool sharedLibMode = true;
#else
  const bool sharedLibMode = false;
#endif
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
  else
  {
    US_DEBUG << "GetLibraryPath_impl() failed for address " << symbol;
  }
  return "";
}

void* GetSymbol_impl(const ModuleInfo& moduleInfo, const char* symbol)
{
  // Clear the last error message
  dlerror();

  void* selfHandle = 0;
  if (!sharedLibMode || moduleInfo.name == "main")
  {
    // Get the handle of the executable
    selfHandle = dlopen(0, RTLD_LAZY);
  }
  else
  {
    selfHandle = dlopen(moduleInfo.location.c_str(), RTLD_LAZY);
  }

  if (selfHandle)
  {
    void* addr = dlsym(selfHandle, symbol);
    if (!addr)
    {
      const char* dlerrorMsg = dlerror();
      if (dlerrorMsg)
      {
        US_DEBUG << "GetSymbol_impl() failed: " << dlerrorMsg;
      }
    }
    dlclose(selfHandle);
    return addr;
  }
  else
  {
    US_DEBUG << "GetSymbol_impl() dlopen() failed: " << dlerror();
  }
  return 0;
}

#elif _WIN32

#include <windows.h>

std::string WstrToUtf8Str(const std::wstring& wstr)
{
  std::string retStr;
  if (!wstr.empty()) {
    int sizeRequired = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);

    if (sizeRequired > 0) {
      std::vector<char> utf8String(sizeRequired);
      int bytesConverted = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(),
        -1, &utf8String[0], (int)utf8String.size(), NULL,
        NULL);
      if (bytesConverted != 0) {
        retStr = &utf8String[0];
      } else {
        // Error
      }
    }
  }
  return retStr;
}

std::wstring Utf8StrToWStr(const std::string& str)
{
  std::wstring retStr;
  if (!str.empty()) {
    int sizeRequired = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

    if (sizeRequired > 0) {
      std::vector<wchar_t> utf16String(sizeRequired);
      int bytesConverted = MultiByteToWideChar(CP_ACP, 0, str.c_str(),
        -1, &utf16String[0], (int)utf16String.size());
      if (bytesConverted != 0) {
        retStr = &utf16String[0];
      } else {
        // Error
      }
    }
  }
  return retStr;
}


std::string GetLibraryPath_impl(void *symbol)
{
  HMODULE handle = 0;
  BOOL handleError = GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                       static_cast<LPCTSTR>(symbol), &handle);
  if (!handleError)
  {
    // Test
    US_DEBUG << "GetLibraryPath_impl():GetModuleHandle() " << GetLastErrorStr();
    return "";
  }

  wchar_t modulePathBytes[512];
  if (GetModuleFileNameW(handle, modulePathBytes, 512))
  {
    std::wstring modulePath = modulePathBytes;
    return WstrToUtf8Str(modulePath);
  }

  US_DEBUG << "GetLibraryPath_impl():GetModuleFileName() " << GetLastErrorStr();
  return "";
}

void* GetSymbol_impl(const ModuleInfo& moduleInfo, const char* symbol)
{
  HMODULE handle = NULL;
  if (!sharedLibMode || moduleInfo.name == "main")
  {
    handle = GetModuleHandle(NULL);
  }
  else
  {
    handle = GetModuleHandleW(Utf8StrToWStr(moduleInfo.location).c_str());
  }

  if (!handle)
  {
    US_DEBUG << "GetSymbol_impl():GetModuleHandle() " << GetLastErrorStr();
    return 0;
  }

  void* addr = (void*)GetProcAddress(handle, symbol);
  if (!addr)
  {
    US_DEBUG << "GetSymbol_impl():GetProcAddress(handle," << symbol << ") " << GetLastErrorStr();
  }
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

US_END_NAMESPACE
