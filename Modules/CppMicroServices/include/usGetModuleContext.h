/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef USGETMODULECONTEXT_H
#define USGETMODULECONTEXT_H

#ifndef US_MODULE_NAME
#error Missing preprocessor define US_MODULE_NAME
#endif

#include <usGlobalConfig.h>
#include <usModuleRegistry.h>
#include <usModule.h>
#include <mitkLog.h>
#include <usModuleUtils_p.h>

namespace us {

class ModuleContext;

/**
 * \ingroup MicroServices
 *
 * \brief Returns the module context of the calling module.
 *
 * This function allows easy access to the ModuleContext instance from
 * inside a C++ Micro Services module.
 *
 * \return The ModuleContext of the calling module.
 */
static inline ModuleContext* GetModuleContext()
{
  Module* module = ModuleRegistry::GetModule(US_STR(US_MODULE_NAME));
  if (module)
  {
    return module->GetModuleContext();
  }
  MITK_WARN << "Module '" << US_STR(US_MODULE_NAME) << "' unknown. "
             "The calling module probably misses a US_MODULE_NAME compile definition "
             "and/or a call to US_INITIALIZE_MODULE in one of its source files.";
  return nullptr;
}

}

#endif // USGETMODULECONTEXT_H
