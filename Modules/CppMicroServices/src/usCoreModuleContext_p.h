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
 * This class is not part of the public API.
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
   * Construct a core context
   *
   */
  CoreModuleContext();

  ~CoreModuleContext();

  void Init();

  void Uninit();

};

}

#endif // USCOREMODULECONTEXT_H
