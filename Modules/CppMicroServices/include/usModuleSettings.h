/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef USMODULESETTINGS_H
#define USMODULESETTINGS_H

#include <usGlobalConfig.h>
#include <MitkCppMicroServicesExports.h>

#include <vector>
#include <string>

namespace us {

/**
 * \ingroup MicroServices
 *
 * \brief Query and set certain properties of the CppMicroServices library.
 *
 * The following environment variables influence the runtime behavior
 * of the CppMicroServices library:
 *
 * - \e US_DISABLE_AUTOLOADING If set, auto-loading of modules is disabled.
 * - \e US_AUTOLOAD_PATHS A ':' (Unix) or ';' (Windows) separated list of paths
 *   from which modules should be auto-loaded.
 *
 * \remarks This class is thread safe.
 */
class MITKCPPMICROSERVICES_EXPORT ModuleSettings
{
public:

  /** \brief A list of file-system paths. */
  typedef std::vector<std::string> PathList;

  /**
   * \brief Returns a special string which can be used as an argument for a
   * AddAutoLoadPath() call.
   *
   * When a module is loaded and this string has been added as a path
   * to the list of auto-load paths the CppMicroServices library will
   * auto-load all modules from the currently being loaded module's
   * auto-load directory.
   *
   * \return A string to be used in AddAutoLoadPath().
   *
   * \remarks The returned string is contained in the default set of
   * auto-load paths, unless a new set of paths is given by a call to
   * SetAutoLoadPaths().
   *
   * \sa MicroServices_AutoLoading
   * \sa US_INITIALIZE_MODULE
   */
  static std::string CURRENT_MODULE_PATH();

  /**
   * \return \c true if threading support has been configured into the
   * CppMicroServices library, \c false otherwise.
   */
  static bool IsThreadingSupportEnabled();

  /**
   * \return \c true if support for module auto-loading is enabled,
   * \c false otherwise.
   *
   * \remarks This method will always return \c false if support for auto-loading
   * has not been configured into the CppMicroServices library or if it has been
   * disabled by defining the US_DISABLE_AUTOLOADING environment variable.
   */
  static bool IsAutoLoadingEnabled();

  /**
   * \brief Enable or disable auto-loading support.
   *
   * \param enable If \c true, enable auto-loading support, disable it otherwise.
   *
   * \remarks Calling this method will have no effect if support for
   * auto-loading has not been configured into the CppMicroServices library of it
   * it has been disabled by defining the US_DISABLE_AUTOLOADING environment variable.
   */
  static void SetAutoLoadingEnabled(bool enable);

  /**
   * \return A list of paths in the file-system from which modules will be
   * auto-loaded.
   */
  static PathList GetAutoLoadPaths();

  /**
   * \brief Set a list of paths in the file-system from which modules should be
   * auto-loaded.
   * \param[in] paths A list of absolute file-system paths.
   */
  static void SetAutoLoadPaths(const PathList& paths);

  /**
   * \brief Add a path in the file-system to the list of paths from which modules
   * will be auto-loaded.
   *
   * \param[in] path The additional absolute auto-load path in the file-system.
   */
  static void AddAutoLoadPath(const std::string& path);

  /**
   * \brief Set a local storage path for persistend module data.
   *
   * This path is used as a base directory for providing modules
   * with a storage path for writing persistent data. The callee
   * must ensure that the provided path exists and is writable.
   *
   * \sa ModuleContext::GetDataFile(const std::string&)
   *
   * \param[in] path An absolute path for writing persistent data.
   */
  static void SetStoragePath(const std::string& path);

  /**
   * \brief Get the absolute path for persistent data. The returned path
   * might be empty. If the path is non-empty, it is safe to assume
   * that the path exists and is writable.
   *
   * \return The absolute path to the persistent storage path.
   */
  static std::string GetStoragePath();

private:

  // purposely not implemented
  ModuleSettings();
  ModuleSettings(const ModuleSettings&);
  ModuleSettings& operator=(const ModuleSettings&);
};

}

#endif // USMODULESETTINGS_H
