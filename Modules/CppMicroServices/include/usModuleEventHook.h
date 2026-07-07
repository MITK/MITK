/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USMODULEEVENTHOOK_H
#define USMODULEEVENTHOOK_H

#include <usServiceInterface.h>
#include <usShrinkableVector.h>

namespace us {

class ModuleContext;
class ModuleEvent;

/**
 * \ingroup MicroServices
 *
 * \brief %Module Event Hook Service.
 *
 * <p>
 * Modules registering this service will be called during module lifecycle
 * (loading, loaded, unloading, and unloaded) operations.
 *
 * \remarks Implementations of this interface are required to be thread-safe.
 */
struct MITKCPPMICROSERVICES_EXPORT ModuleEventHook
{

  /** \brief Virtual destructor. */
  virtual ~ModuleEventHook();

  /**
   * \brief Module event hook method. This method is called prior to module event
   * delivery when a module is loading, loaded, unloading, or unloaded.
   * This method can filter the modules which receive the event.
   * <p>
   * This method is called one and only one time for
   * each module event generated, this includes module events which are
   * generated when there are no module listeners registered.
   *
   * \param[in] event The module event to be delivered.
   * \param[in] contexts A list of Module Contexts for modules which have
   *        listeners to which the specified event will be delivered. The
   *        implementation of this method may remove module contexts from the
   *        list to prevent the event from being delivered to the
   *        associated modules.
   */
  virtual void Event(const ModuleEvent& event, ShrinkableVector<ModuleContext*>& contexts) = 0;
};

}

#endif // USMODULEEVENTHOOK_H
