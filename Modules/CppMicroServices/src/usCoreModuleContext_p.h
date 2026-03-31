/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef USCOREMODULECONTEXT_H
#define USCOREMODULECONTEXT_H

#include "usServiceListeners_p.h"
#include "usServiceRegistry_p.h"
#include "usModuleHooks_p.h"
#include "usServiceHooks_p.h"

namespace us {

/**
 * \brief Internal context holding shared framework state for all modules.
 *
 * This class is not part of the public API.
 *
 * \sa ServiceRegistry ServiceListeners ServiceHooks ModuleHooks
 */
class CoreModuleContext
{
public:

  /**
   * All listeners in this framework.
   */
  ServiceListeners listeners;

  /**
   * All registered services in this framework.
   */
  ServiceRegistry services;

  /**
   * All service hooks.
   */
  ServiceHooks serviceHooks;

  /**
   * All module hooks.
   */
  ModuleHooks moduleHooks;

  /**
   * \brief Construct a core context.
   */
  CoreModuleContext();

  /** \brief Destructor. */
  ~CoreModuleContext();

  /** \brief Initialize all sub-components (listeners, services, hooks). */
  void Init();

  /** \brief Tear down all sub-components. */
  void Uninit();

};

}

#endif // USCOREMODULECONTEXT_H
