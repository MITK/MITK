/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkModuleUtils.h"
#include <mitkLogMacros.h>

namespace mitk {

#ifdef __GNUC__

#define _GNU_SOURCE
#include <dlfcn.h>

std::string GetLibraryPath_impl(const std::string& /*libName*/, const void* symbol)
{
  Dl_info info;
  if (dladdr(symbol, &info))
  {
    return info.dli_fname;
  }
  return "";
}

void* GetSymbol_impl(const std::string& libName, const char* symbol)
{
  void* selfHandle = 0;
  if (libName.empty())
  {
    // Get the handle of the executable
    selfHandle = dlopen(0, RTLD_LAZY);
  }
  else
  {
    selfHandle = dlopen(libName.c_str(), RTLD_LAZY);
  }

  if (selfHandle)
  {
    void* addr = dlsym(selfHandle, symbol);
    dlclose(selfHandle);
    return addr;
  }
  return 0;
}

#elif _WIN32

#include <windows.h>
#include <strsafe.h>

void PrintLastError_impl(const std::string& context)
{
  // Retrieve the system error message for the last-error code
      LPVOID lpMsgBuf;
      LPVOID lpDisplayBuf;
      DWORD dw = GetLastError(); 

      FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

        // Display the error message and exit the process

      lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)context.c_str()) + 50) * sizeof(TCHAR)); 
      StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("Error in context %s (%d): %s"), 
        context.c_str(), dw, lpMsgBuf); 

      std::string errMsg((LPCTSTR)lpDisplayBuf);

      LocalFree(lpMsgBuf);
      LocalFree(lpDisplayBuf);

      MITK_DEBUG(true) << errMsg;
}

std::string GetLibraryPath_impl(const std::string& libName, const void *symbol)
{
  HMODULE handle = 0;
  if (libName.empty())
  {
    // get the handle for the executable
    handle = GetModuleHandle(0);
  }
  else
  {
    handle = GetModuleHandle(libName.c_str());
  }
  if (!handle)
  {
    PrintLastError_impl("GetLibraryPath_impl():GetModuleHandle()");
    return "";
  }

  char modulePath[512];
  if (GetModuleFileName(handle, modulePath, 512))
  {
    return modulePath;
  }

  PrintLastError_impl("GetLibraryPath_impl():GetModuleFileName()");
  return "";
}

void* GetSymbol_impl(const std::string& libName, const char* symbol)
{
  HMODULE handle = 0;
  if (libName.empty())
  {
    // Get the handle of the executable
    handle = GetModuleHandle(NULL);
  }
  else
  {
    handle = GetModuleHandle(libName.c_str());
  }

  if (!handle)
  {
    PrintLastError_impl("GetSymbol_impl():GetModuleHandle()");
    return 0;
  }

  void* addr = (void*)GetProcAddress(handle, symbol);
  if (!addr)
  {
    PrintLastError_impl(std::string("GetSymbol_impl():GetProcAddress(handle,") + std::string(symbol) + ")");
  }
  return addr;
}
#else
std::string GetLibraryPath_impl(const std::string& libName, const void* symbol)
{
  return "";
}

void* GetSymbol_impl(const std::string& libName, const char* symbol)
{
  return 0;
}
#endif

std::string ModuleUtils::GetLibraryPath(const std::string& libName, const void* symbol)
{
  return GetLibraryPath_impl(libName, symbol);
}

void* ModuleUtils::GetSymbol(const std::string& libName, const char* symbol)
{
  return GetSymbol_impl(libName, symbol);
}

}
