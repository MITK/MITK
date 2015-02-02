/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <string>
#include <stdexcept>

#include "mitkTestingConfig.h"

#if defined(__APPLE__)
  #define PLATFORM_APPLE
#endif

#if defined(unix) || defined(__unix) || defined(__APPLE__)
  #include <dlfcn.h>
  #define PLATFORM_UNIX
#elif defined(_WIN32)
  #include <Windows.h>
  #include <strsafe.h>
  #define PLATFORM_WINDOWS
#else
  #error Unsupported platform
#endif


namespace mitk {

class SharedLibraryHandle
{
public:

  SharedLibraryHandle() : m_Handle(0) {}

  SharedLibraryHandle(const std::string& name)
    : m_Name(name), m_Handle(0)
  {}

  virtual ~SharedLibraryHandle() {}

  void Load()
  {
    Load(m_Name);
  }

  void Load(const std::string& name)
  {
    if (m_Handle) throw std::logic_error(std::string("Library already loaded: ") + name);
    std::string libPath = GetAbsolutePath(name);
#ifdef PLATFORM_UNIX
    m_Handle = dlopen(libPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (!m_Handle)
    {
     const char* err = dlerror();
     throw std::runtime_error(err ? std::string(err) : libPath);
    }
#else
    m_Handle = LoadLibrary(libPath.c_str());
    if (!m_Handle)
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
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)libPath.c_str()) + 50) * sizeof(TCHAR));
      StringCchPrintf((LPTSTR)lpDisplayBuf,
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("Loading %s failed with error %d: %s"),
        libPath.c_str(), dw, lpMsgBuf);

      std::string errMsg((LPCTSTR)lpDisplayBuf);

      LocalFree(lpMsgBuf);
      LocalFree(lpDisplayBuf);

      throw std::runtime_error(errMsg);
    }
#endif

    m_Name = name;
  }

  void Unload()
  {
    if (m_Handle)
    {
#ifdef PLATFORM_UNIX
     dlclose(m_Handle);
#else
      FreeLibrary((HMODULE)m_Handle);
#endif
     m_Handle = 0;
    }
  }

  std::string GetAbsolutePath(const std::string& name)
  {
    return GetLibraryPath() + "/" + Prefix() + name + Suffix();
  }

  std::string GetAbsolutePath()
  {
    return GetLibraryPath() + "/" + Prefix() + m_Name + Suffix();
  }

  static std::string GetLibraryPath()
  {
    return std::string(MITK_RUNTIME_OUTPUT_DIR);
  }

  static std::string Suffix()
  {
#ifdef PLATFORM_WINDOWS
    return ".dll";
#elif defined(PLATFORM_APPLE)
    return ".dylib";
#else
    return ".so";
#endif
  }

  static std::string Prefix()
  {
#if defined(PLATFORM_UNIX)
    return "lib";
#else
    return "";
#endif
  }

private:

  SharedLibraryHandle(const SharedLibraryHandle&);
  SharedLibraryHandle& operator = (const SharedLibraryHandle&);

  std::string m_Name;
  void* m_Handle;
};


} // namespace mitk

