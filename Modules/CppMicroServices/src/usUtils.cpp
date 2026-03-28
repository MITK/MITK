/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usUtils_p.h>

#include <mitkLog.h>
#include <usModuleInfo.h>
#include <usModuleSettings.h>

#include <string>
#include <cstdio>
#include <cctype>
#include <algorithm>
#include <typeinfo>

#include <filesystem>

#ifdef US_PLATFORM_POSIX
  #include <cerrno>
  #include <cstring>
  #include <dlfcn.h>
#else
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
  #include <crtdbg.h>
#endif

#if defined(__GNUC__) || defined(__clang__)
  #include <cxxabi.h>
#endif

//-------------------------------------------------------------------
// Module auto-loading
//-------------------------------------------------------------------

namespace {

#if !defined(US_PLATFORM_LINUX)
std::string library_suffix()
{
#ifdef US_PLATFORM_WINDOWS
  return ".dll";
#elif defined(US_PLATFORM_APPLE)
  return ".dylib";
#else
  return ".so";
#endif
}
#endif

#ifdef US_PLATFORM_POSIX

const char DIR_SEP = '/';

bool load_impl(const std::string& modulePath)
{
  void* handle = dlopen(modulePath.c_str(), RTLD_NOW | RTLD_LOCAL);
  if (handle == nullptr)
  {
    MITK_WARN << dlerror();
  }
  return (handle != nullptr);
}

#elif defined(US_PLATFORM_WINDOWS)

const char DIR_SEP = '\\';

bool load_impl(const std::string& modulePath)
{
  void* handle = LoadLibrary(modulePath.c_str());
  if (handle == nullptr)
  {
    MITK_WARN << us::GetLastErrorStr();
  }
  return (handle != nullptr);
}

#else

  #error "Missing load_impl implementation for this platform."

#endif

}

namespace us {

std::vector<std::string> AutoLoadModulesFromPath(const std::string& absoluteBasePath, const std::string& subDir)
{
  std::vector<std::string> loadedModules;

  namespace fs = std::filesystem;

  fs::path loadPath = fs::path(absoluteBasePath) / subDir;

#ifdef CMAKE_INTDIR
  // Try intermediate output directories
  if (!fs::is_directory(loadPath))
  {
    fs::path basePath(absoluteBasePath);
    std::string intermediateDir = basePath.filename().string();
    bool equalSubDir = intermediateDir.size() == std::strlen(CMAKE_INTDIR);
    for (std::size_t i = 0; equalSubDir && i < intermediateDir.size(); ++i)
    {
      if (std::tolower(intermediateDir[i]) != std::tolower(CMAKE_INTDIR[i]))
      {
        equalSubDir = false;
      }
    }
    if (equalSubDir)
    {
      loadPath = basePath.parent_path() / subDir / CMAKE_INTDIR;
    }
  }
#endif

  std::error_code ec;
  for (const auto& entry : fs::directory_iterator(loadPath, ec))
  {
    if (!entry.is_regular_file(ec))
      continue;

    std::string entryFileName = entry.path().filename().string();

    // On Linux, library file names can have version numbers appended. On other platforms, we
    // check the file ending. This could be refined for Linux in the future.
#if !defined(US_PLATFORM_LINUX)
    if (entryFileName.rfind(library_suffix()) != (entryFileName.size() - library_suffix().size()))
    {
      continue;
    }
#endif

    std::string libPath = entry.path().string();

    if (!load_impl(libPath))
    {
      MITK_WARN << "Auto-loading of module " << libPath << " failed.";
    }
    else
    {
      loadedModules.push_back(libPath);
    }
  }
  return loadedModules;
}

std::vector<std::string> AutoLoadModules(const ModuleInfo& moduleInfo)
{
  std::vector<std::string> loadedModules;

  if (moduleInfo.autoLoadDir.empty())
  {
    return loadedModules;
  }

  ModuleSettings::PathList autoLoadPaths = ModuleSettings::GetAutoLoadPaths();

  std::size_t indexOfLastSeparator = moduleInfo.location.find_last_of(DIR_SEP);
  std::string moduleBasePath = moduleInfo.location.substr(0, indexOfLastSeparator);

  for (ModuleSettings::PathList::iterator i = autoLoadPaths.begin();
       i != autoLoadPaths.end(); ++i)
  {
    if (*i == ModuleSettings::CURRENT_MODULE_PATH())
    {
      // Load all modules from a directory located relative to this modules location
      // and named after this modules library name.
      *i = moduleBasePath;
    }
  }

  // We could have introduced a duplicate above, so remove it.
  std::sort(autoLoadPaths.begin(), autoLoadPaths.end());
  autoLoadPaths.erase(std::unique(autoLoadPaths.begin(), autoLoadPaths.end()), autoLoadPaths.end());
  for (ModuleSettings::PathList::iterator i = autoLoadPaths.begin();
       i != autoLoadPaths.end(); ++i)
  {
    if (i->empty()) continue;
    std::vector<std::string> paths = AutoLoadModulesFromPath(*i, moduleInfo.autoLoadDir);
    loadedModules.insert(loadedModules.end(), paths.begin(), paths.end());
  }
  return loadedModules;
}

}

//-------------------------------------------------------------------
// Error handling
//-------------------------------------------------------------------

namespace us {

std::string GetLastErrorStr()
{
#ifdef US_PLATFORM_POSIX
  return std::string(strerror(errno));
#else
  // Retrieve the system error message for the last-error code
  LPVOID lpMsgBuf;
  DWORD dw = GetLastError();

  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    nullptr,
    dw,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPTSTR) &lpMsgBuf,
    0, nullptr );

  std::string errMsg((LPCTSTR)lpMsgBuf);

  LocalFree(lpMsgBuf);

  return errMsg;
#endif
}

MITKCPPMICROSERVICES_EXPORT ::std::string GetDemangledName(const ::std::type_info& typeInfo)
{
  ::std::string result;
#if defined(__GNUC__) || defined(__clang__)
  int status = 0;
  char* demangled = abi::__cxa_demangle(typeInfo.name(), nullptr, nullptr, &status);
  if (demangled && status == 0)
  {
    result = demangled;
    free(demangled);
  }
#elif defined(US_PLATFORM_WINDOWS)
  const char* demangled = typeInfo.name();
  if (demangled != nullptr)
  {
    result = demangled;
    // remove "struct" qualifiers
    std::size_t pos = 0;
    while (pos != std::string::npos)
    {
      if ((pos = result.find("struct ", pos)) != std::string::npos)
      {
        result = result.substr(0, pos) + result.substr(pos + 7);
        pos += 8;
      }
    }
    // remove "class" qualifiers
    pos = 0;
    while (pos != std::string::npos)
    {
      if ((pos = result.find("class ", pos)) != std::string::npos)
      {
        result = result.substr(0, pos) + result.substr(pos + 6);
        pos += 7;
      }
    }
  }
#else
  (void)typeInfo;
#endif
  return result;
}

}
