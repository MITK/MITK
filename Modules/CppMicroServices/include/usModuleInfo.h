/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef USMODULEINFO_H
#define USMODULEINFO_H

#include <usGlobalConfig.h>
#include <MitkCppMicroServicesExports.h>

#include <string>

namespace us {

struct ModuleActivator;

/**
 * \brief This class is not intended to be used directly. It is exported to support
 * the CppMicroServices module system.
 */
struct MITKCPPMICROSERVICES_EXPORT ModuleInfo
{
  /** \brief Construct a ModuleInfo with the given name.
   * \param[in] name The module name.
   */
  ModuleInfo(const std::string& name);

  std::string name;       ///< \brief The module name.
  std::string location;   ///< \brief The file system location of the module.
  std::string autoLoadDir; ///< \brief The auto-load directory for the module.
  long id;                ///< \brief The unique module identifier.
};

}

#endif // USMODULEINFO_H
