/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usModuleSettings.h>
#include <usThreads_p.h>
#include <usStaticInit_p.h>

#include <string>
#include <sstream>
#include <set>
#include <algorithm>
#include <cctype>

namespace us {

namespace {

  std::string RemoveTrailingPathSeparator(const std::string& in)
  {
#ifdef US_PLATFORM_WINDOWS
    const char separator = '\\';
#else
    const char separator = '/';
#endif
    if (in.empty()) return in;
    std::string::const_iterator lastChar = --in.end();
    while (lastChar != in.begin() && std::isspace(*lastChar)) lastChar--;
    if (*lastChar != separator) lastChar++;
    std::string::const_iterator firstChar = in.begin();
    while (firstChar < lastChar && std::isspace(*firstChar)) firstChar++;
    return std::string(firstChar, lastChar);
  }

}

std::string ModuleSettings::CURRENT_MODULE_PATH()
{
  static const std::string var = "us_current_module_path";
  return var;
}

struct ModuleSettingsPrivate : public MultiThreaded<>
{
  ModuleSettingsPrivate()
    : autoLoadPaths()
    , autoLoadingEnabled(true)
    , autoLoadingDisabled(false)
  {
    autoLoadPaths.insert(ModuleSettings::CURRENT_MODULE_PATH());

    char* envPaths = getenv("US_AUTOLOAD_PATHS");
    if (envPaths != nullptr)
    {
      std::stringstream ss(envPaths);
      std::string envPath;
#ifdef US_PLATFORM_WINDOWS
      const char separator = ';';
#else
      const char separator = ':';
#endif
      while (std::getline(ss, envPath, separator))
      {
        std::string normalizedEnvPath = RemoveTrailingPathSeparator(envPath);
        if (!normalizedEnvPath.empty())
        {
          extraPaths.insert(normalizedEnvPath);
        }
      }
    }

    if (getenv("US_DISABLE_AUTOLOADING"))
    {
      autoLoadingDisabled = true;
    }
  }

  std::set<std::string> autoLoadPaths;
  std::set<std::string> extraPaths;
  bool autoLoadingEnabled;
  bool autoLoadingDisabled;
  std::string storagePath;
};

US_GLOBAL_STATIC(ModuleSettingsPrivate, moduleSettingsPrivate)

bool ModuleSettings::IsThreadingSupportEnabled()
{
  return true;
}

bool ModuleSettings::IsAutoLoadingEnabled()
{
  ModuleSettingsPrivate::Lock lock(moduleSettingsPrivate());
  return !moduleSettingsPrivate()->autoLoadingDisabled &&
      moduleSettingsPrivate()->autoLoadingEnabled;
}

void ModuleSettings::SetAutoLoadingEnabled(bool enable)
{
  ModuleSettingsPrivate::Lock lock(moduleSettingsPrivate());
  moduleSettingsPrivate()->autoLoadingEnabled = enable;
}

ModuleSettings::PathList ModuleSettings::GetAutoLoadPaths()
{
  ModuleSettingsPrivate::Lock lock(moduleSettingsPrivate());
  ModuleSettings::PathList paths(moduleSettingsPrivate()->autoLoadPaths.begin(),
                                 moduleSettingsPrivate()->autoLoadPaths.end());
  paths.insert(paths.end(), moduleSettingsPrivate()->extraPaths.begin(),
               moduleSettingsPrivate()->extraPaths.end());
  std::sort(paths.begin(), paths.end());
  paths.erase(std::unique(paths.begin(), paths.end()), paths.end());
  return paths;
}

void ModuleSettings::SetAutoLoadPaths(const PathList& paths)
{
  PathList normalizedPaths;
  normalizedPaths.resize(paths.size());
  std::transform(paths.begin(), paths.end(), normalizedPaths.begin(), RemoveTrailingPathSeparator);

  ModuleSettingsPrivate::Lock lock(moduleSettingsPrivate());
  moduleSettingsPrivate()->autoLoadPaths.clear();
  moduleSettingsPrivate()->autoLoadPaths.insert(normalizedPaths.begin(), normalizedPaths.end());
}

void ModuleSettings::AddAutoLoadPath(const std::string& path)
{
  ModuleSettingsPrivate::Lock lock(moduleSettingsPrivate());
  moduleSettingsPrivate()->autoLoadPaths.insert(RemoveTrailingPathSeparator(path));
}

void ModuleSettings::SetStoragePath(const std::string &path)
{
  ModuleSettingsPrivate::Lock lock(moduleSettingsPrivate());
  moduleSettingsPrivate()->storagePath = RemoveTrailingPathSeparator(path);
}

std::string ModuleSettings::GetStoragePath()
{
  ModuleSettingsPrivate::Lock lock(moduleSettingsPrivate());
  return moduleSettingsPrivate()->storagePath;
}

}
