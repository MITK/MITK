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
 * This class is not intended to be used directly. It is exported to support
 * the CppMicroServices module system.
 */
struct MITKCPPMICROSERVICES_EXPORT ModuleInfo
{
  ModuleInfo(const std::string& name);

  std::string name;
  std::string location;
  std::string autoLoadDir;
  long id;
};

}

#endif // USMODULEINFO_H
