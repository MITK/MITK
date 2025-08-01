/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkEnvironment_h
#define mitkEnvironment_h

#include <mitkFileSystem.h>
#include <MitkCoreExports.h>

#include <optional>
#include <string>

namespace mitk
{
#if defined(_WIN32)
  inline constexpr char PATH_ENV_SEPARATOR = ';';
#else
  inline constexpr char PATH_ENV_SEPARATOR = ':';
#endif

  MITKCORE_EXPORT std::optional<std::string> GetEnv(const std::string& name);

  MITKCORE_EXPORT bool SetEnv(const std::string& name, const std::string& value);

  MITKCORE_EXPORT bool UnsetEnv(const std::string& name);

  MITKCORE_EXPORT bool AddPathEnv(const fs::path& pathEntry);

  MITKCORE_EXPORT bool RemovePathEnv(const fs::path& pathEntry);
}

#endif
