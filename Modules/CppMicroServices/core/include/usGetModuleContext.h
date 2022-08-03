/*============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center (DKFZ)
  All rights reserved.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

============================================================================*/


#ifndef USGETMODULECONTEXT_H
#define USGETMODULECONTEXT_H

#ifndef US_MODULE_NAME
#error Missing preprocessor define US_MODULE_NAME
#endif

#include <usGlobalConfig.h>
#include <usModuleRegistry.h>
#include <usModule.h>
#include <usLog_p.h>
#include <usModuleUtils_p.h>

US_BEGIN_NAMESPACE

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
  US_WARN << "Module '" << US_STR(US_MODULE_NAME) << "' unknown. "
             "The calling module probably misses a US_MODULE_NAME compile definition "
             "and/or a call to US_INITIALIZE_MODULE in one of its source files.";
  return nullptr;
}

US_END_NAMESPACE

#endif // USGETMODULECONTEXT_H
