/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USMODULEHOOKS_P_H
#define USMODULEHOOKS_P_H

#include "usServiceListeners_p.h"

#include <vector>

namespace us {

class CoreModuleContext;
class Module;
class ModuleContext;
class ModuleEvent;

class ModuleHooks
{

private:

  CoreModuleContext* const coreCtx;

public:

  ModuleHooks(CoreModuleContext* ctx);

  Module* FilterModule(const ModuleContext* mc, Module* module) const;

  void FilterModules(const ModuleContext* mc, std::vector<Module*>& modules) const;

  void FilterModuleEventReceivers(const ModuleEvent& evt,
                                  ServiceListeners::ModuleListenerMap& moduleListeners);

};

}

#endif // USMODULEHOOKS_P_H
