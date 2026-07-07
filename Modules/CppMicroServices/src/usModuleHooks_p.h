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

/**
 * \brief Internal helper that invokes registered module hook services.
 *
 * This class is not part of the public API.
 *
 * \sa CoreModuleContext ServiceHooks
 */
class ModuleHooks
{

private:

  CoreModuleContext* const coreCtx; ///< \brief Owning core context.

public:

  /**
   * \brief Construct ModuleHooks bound to the given core context.
   *
   * \param[in] ctx The core module context that owns the hooks.
   */
  ModuleHooks(CoreModuleContext* ctx);

  /**
   * \brief Apply FindHook services to filter a single module.
   *
   * \param[in] mc The calling module context.
   * \param[in] module The module to potentially filter.
   * \return The module, or \c nullptr if filtered out by a hook.
   */
  Module* FilterModule(const ModuleContext* mc, Module* module) const;

  /**
   * \brief Apply FindHook services to filter a vector of modules.
   *
   * \param[in] mc The calling module context.
   * \param[in,out] modules The module vector; entries may be removed by hooks.
   */
  void FilterModules(const ModuleContext* mc, std::vector<Module*>& modules) const;

  /**
   * \brief Apply EventListenerHook services to filter module event receivers.
   *
   * \param[in] evt The module event being dispatched.
   * \param[in,out] moduleListeners The listener map; entries may be removed by hooks.
   */
  void FilterModuleEventReceivers(const ModuleEvent& evt,
                                  ServiceListeners::ModuleListenerMap& moduleListeners);

};

}

#endif // USMODULEHOOKS_P_H
