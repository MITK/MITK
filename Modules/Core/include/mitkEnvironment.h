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

  /**
   * \defgroup MITKEnvironment Environment variable utilities
   * \brief Functions for querying and modifying environment variables in MITK.
   *
   * These functions provide a platform-independent interface for retrieving,
   * setting, and removing environment variables, as well as manipulating
   * the `PATH` variable.
   */

  /**
   * \brief Retrieve the value of an environment variable.
   *
   * This function looks up the given environment variable name and returns
   * its value if it exists.
   *
   * \param name Name of the environment variable.
   * \return The value of the environment variable if set, or `std::nullopt` if not found.
   *
   * \ingroup MITKEnvironment
   */
  MITKCORE_EXPORT std::optional<std::string> GetEnv(const std::string& name);

  /**
   * \brief Set the value of an environment variable.
   *
   * If the environment variable already exists, its value is overwritten.
   *
   * \param name Name of the environment variable.
   * \param value New value for the environment variable.
   * \return `true` if the variable was successfully set, `false` otherwise.
   *
   * \ingroup MITKEnvironment
   */
  MITKCORE_EXPORT bool SetEnv(const std::string& name, const std::string& value);

  /**
   * \brief Remove an environment variable.
   *
   * The variable is removed from the current process environment.
   *
   * \param name Name of the environment variable to remove.
   * \return `true` if the variable was successfully removed, `false` otherwise.
   *
   * \ingroup MITKEnvironment
   */
  MITKCORE_EXPORT bool UnsetEnv(const std::string& name);

  /**
   * \brief Add a directory to the PATH environment variable.
   *
   * The path is normalized before being added. If it already exists in PATH,
   * no changes are made.
   *
   * \param pathEntry Directory to add to PATH.
   * \return `true` if the path was added or already existed, `false` if the operation failed.
   *
   * \ingroup MITKEnvironment
   */
  MITKCORE_EXPORT bool AddPathEnv(const fs::path& pathEntry);

  /**
   * \brief Remove a directory from the PATH environment variable.
   *
   * The path is normalized before comparison. If the path is not present,
   * no changes are made.
   *
   * \param pathEntry Directory to remove from PATH.
   * \return `true` if the path was removed or was not present, `false` if the operation failed.
   *
   * \ingroup MITKEnvironment
   */
  MITKCORE_EXPORT bool RemovePathEnv(const fs::path& pathEntry);
}

#endif
