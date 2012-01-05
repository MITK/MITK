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

#ifndef MITKMODULEREGISTRY_H
#define MITKMODULEREGISTRY_H

#include <vector>
#include <string>

#include <MitkExports.h>

namespace mitk {

class Module;
struct ModuleInfo;

/**
 * \ingroup MicroServices
 *
 * Here we handle all the modules that are loaded in the framework.
 */
class MITK_CORE_EXPORT ModuleRegistry {

public:

  /**
   * Get the module that has the specified module identifier.
   *
   * @param id The identifier of the module to get.
   * @return Module or null
   *         if the module was not found.
   */
  static Module* GetModule(long id);

  /**
   * Get the module that has specified module name.
   *
   * @param name The name of the module to get.
   * @return Module or null.
   */
  static Module* GetModule(const std::string& name);

  /**
   * Get all known modules.
   *
   * @return A Module list with modules.
   */
  static void GetModules(std::vector<Module*>& modules);

  /**
   * Get all modules currently in module state LOADED.
   *
   * @return A List of Modules.
   */
  static void GetLoadedModules(std::vector<Module*>& modules);

private:

  friend class ModuleInitializer;

  // disabled
  ModuleRegistry();

  static void Register(ModuleInfo* info);

  static void UnRegister(const ModuleInfo* info);

};

}


#endif // MITKMODULEREGISTRY_H
