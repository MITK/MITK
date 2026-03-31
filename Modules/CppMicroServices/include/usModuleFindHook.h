/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USMODULEFINDHOOK_H
#define USMODULEFINDHOOK_H

#include <usServiceInterface.h>
#include <usShrinkableVector.h>

namespace us {

class Module;
class ModuleContext;

/**
 * @ingroup MicroServices
 *
 * %Module Context Hook Service.
 *
 * <p>
 * Modules registering this service will be called during module find
 * (get modules) operations.
 *
 * @remarks Implementations of this interface are required to be thread-safe.
 */
struct MITKCPPMICROSERVICES_EXPORT ModuleFindHook
{

  virtual ~ModuleFindHook();

  /**
   * Find hook method. This method is called for module find operations
   * using ModuleContext::GetBundle(long)
   * and ModuleContext::GetModules() methods. The find method can
   * filter the result of the find operation.
   *
   * \note A find operation using the ModuleContext::GetModule(const std::string&)
   *       method does not cause the find method to be called, neither does any
   *       call to the static methods of the ModuleRegistry class.
   *
   * @param context The module context of the module performing the find
   *        operation.
   * @param modules A list of Modules to be returned as a result of the
   *        find operation. The implementation of this method may remove
   *        modules from the list to prevent the modules from being
   *        returned to the module performing the find operation.
   */
  virtual void Find(const ModuleContext* context, ShrinkableVector<Module*>& modules) = 0;
};

}

#endif // USMODULEFINDHOOK_H
