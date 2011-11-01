/*=============================================================================

  Library: CTK

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/

#include "mitkStaticInit.h"

#include "mitkModuleRegistry.h"
#include "mitkModuleContext.h"
#include "mitkModule.h"
#include "mitkModuleInfo.h"
#include "mitkModuleUtils.h"

namespace mitk {

MITK_GLOBAL_STATIC_WITH_ARGS(ModuleInfo, moduleInfo, ("@MODULE_NAME@", "@MODULE_LIBNAME@", "@MODULE_DEPENDS_STR@", "@MODULE_PACKAGE_DEPENDS_STR@", "@MODULE_VERSION@", @MODULE_QT_BOOL@))

class MITK_LOCAL ModuleInitializer {

public:

  ModuleInitializer()
  {
    std::string location = ModuleUtils::GetLibraryPath(moduleInfo()->libName, (void*)moduleInfo);
    std::string activator_func = "_mitk_module_activator_instance_";
    activator_func.append(moduleInfo()->name);

    moduleInfo()->location = location;

    if (moduleInfo()->libName.empty())
    {
      // make sure we retrieve symbols from the executable, if "libName" is empty
      location.clear();
    }
    moduleInfo()->activatorHook = (ModuleInfo::ModuleActivatorHook)ModuleUtils::GetSymbol(location, activator_func.c_str());

    Register();
  }

  static void Register()
  {
    ModuleRegistry::Register(moduleInfo());
  }

  ~ModuleInitializer()
  {
    ModuleRegistry::UnRegister(moduleInfo());
  }

};

ModuleContext* GetModuleContext()
{
  // make sure the module is registered
  if (moduleInfo()->id == 0)
  {
    ModuleInitializer::Register();
  }

  return ModuleRegistry::GetModule(moduleInfo()->id)->GetModuleContext();
}

}

static mitk::ModuleInitializer coreModule;



