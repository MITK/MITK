/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef USMODULEUTILS_H
#define USMODULEUTILS_H

#include <usGlobalConfig.h>
#include <MitkCppMicroServicesExports.h>

#include <string>

#define US_STR_(x) #x
#define US_STR(x) US_STR_(x)
#define US_CONCAT_(x,y) x ## y
#define US_CONCAT(x,y) US_CONCAT_(x,y)

namespace us {

struct ModuleInfo;

/**
 * This class is not intended to be used directly. It is exported to support
 * the CppMicroServices module system.
 */
struct MITKCPPMICROSERVICES_EXPORT ModuleUtils
{
  static std::string GetLibraryPath(void* symbol);

  static void* GetSymbol(const ModuleInfo& module, const char* symbol);
};

}

#endif // USMODULEUTILS_H
