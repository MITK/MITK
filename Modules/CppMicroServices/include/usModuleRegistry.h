/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USMODULEREGISTRY_H
#define USMODULEREGISTRY_H

#include <vector>
#include <string>

#include <usGlobalConfig.h>
#include <MitkCppMicroServicesExports.h>

namespace us {

class Module;
struct ModuleInfo;
struct ModuleActivator;

/**
 * \ingroup MicroServices
 *
 * \brief Here we handle all the modules that are loaded in the framework.
 */
class MITKCPPMICROSERVICES_EXPORT ModuleRegistry {

public:

  /**
   * \brief Get the module that has the specified module identifier.
   *
   * \param[in] id The identifier of the module to get.
   * \return Module or null
   *         if the module was not found.
   */
  static Module* GetModule(long id);

  /**
   * \brief Get the module that has specified module name.
   *
   * \param[in] name The name of the module to get.
   * \return Module or null.
   */
  static Module* GetModule(const std::string& name);

  /**
   * \brief Get all known modules.
   *
   * \return A list which is filled with all known modules.
   */
  static std::vector<Module*> GetModules();

  /**
   * \brief Get all modules currently in module state <code>LOADED</code>.
   *
   * \return A list which is filled with all modules in
   *         state <code>LOADED</code>
   */
  static std::vector<Module*> GetLoadedModules();

  /** \brief Register a module with the framework.
   * \param[in] info The module information to register.
   */
  static void Register(ModuleInfo* info);

  /** \brief Unregister a module from the framework.
   * \param[in] info The module information to unregister.
   */
  static void UnRegister(const ModuleInfo* info);

private:

  // disabled
  ModuleRegistry();

};

}

#endif // USMODULEREGISTRY_H
