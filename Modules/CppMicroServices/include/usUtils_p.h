/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef USUTILS_H
#define USUTILS_H

#include <usGlobalConfig.h>
#include <MitkCppMicroServicesExports.h>

#include <string>
#include <vector>

//-------------------------------------------------------------------
// Module auto-loading
//-------------------------------------------------------------------

namespace us {

struct ModuleInfo;

std::vector<std::string> AutoLoadModules(const ModuleInfo& moduleInfo);

}

//-------------------------------------------------------------------
// Error handling
//-------------------------------------------------------------------

namespace us {

MITKCPPMICROSERVICES_EXPORT std::string GetLastErrorStr();

}

#endif // USUTILS_H
