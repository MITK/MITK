/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkEnvironment.h>

#include <cstdlib>
#include <vector>

namespace
{
  std::vector<std::string> Split(const std::string& str, char delim)
  {
    std::vector<std::string> tokens;
    size_t start = 0, delimPos;

    while ((delimPos = str.find(delim, start)) != std::string::npos)
    {
      tokens.emplace_back(str.substr(start, delimPos - start));
      start = delimPos + 1;
    }

    tokens.emplace_back(str.substr(start));
    return tokens;
  }

  std::string Join(const std::vector<std::string>& tokens, char delim)
  {
    if (tokens.empty())
      return "";

    std::string result = tokens.front();

    for (size_t i = 1; i < tokens.size(); ++i)
    {
      result += delim;
      result += tokens[i];
    }

    return result;
  }
}

std::optional<std::string> mitk::GetEnv(const std::string& name)
{
  auto var = std::getenv(name.c_str());

  if (var == nullptr)
    return std::nullopt;

  return var;
}

bool mitk::SetEnv(const std::string& name, const std::string& value)
{
#if defined(_WIN32)
  return _putenv_s(name.c_str(), value.c_str()) == 0;
#else
  return setenv(name.c_str(), value.c_str(), 1) == 0;
#endif
}

bool mitk::UnsetEnv(const std::string& name)
{
#if defined(_WIN32)
  return _putenv_s(name.c_str(), "") == 0;
#else
  return unsetenv(name.c_str()) == 0;
#endif
}

bool mitk::AddPathEnv(const fs::path& pathEntry)
{
  if (pathEntry.empty())
    return true;

  const auto pathEntryAsString = pathEntry.lexically_normal().string();
  const auto oldPathEnv = GetEnv("PATH").value_or("");

  if (!oldPathEnv.empty())
  {
    const auto paths = Split(oldPathEnv, PATH_ENV_SEPARATOR);

    for (const auto& path : paths)
    {
      if (path == pathEntryAsString)
        return true;
    }
  }

  auto newPathEnv = pathEntryAsString;

  if (!oldPathEnv.empty())
    newPathEnv += PATH_ENV_SEPARATOR + oldPathEnv;

  return SetEnv("PATH", newPathEnv);
}

bool mitk::RemovePathEnv(const fs::path& pathEntry)
{
  if (pathEntry.empty())
    return true;

  const auto pathEntryAsString = pathEntry.lexically_normal().string();
  const auto oldPathEnv = GetEnv("PATH").value_or("");

  if (oldPathEnv.empty())
    return true;

  auto newPaths = Split(oldPathEnv, PATH_ENV_SEPARATOR);
  const auto oldSize = newPaths.size();

  newPaths.erase(std::remove(newPaths.begin(), newPaths.end(), pathEntryAsString), newPaths.end());

  if (newPaths.size() == oldSize)
    return true;

  const auto newPathEnv = Join(newPaths, PATH_ENV_SEPARATOR);
  return SetEnv("PATH", newPathEnv);
}
