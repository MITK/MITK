/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USSERVICEEVENTLISTENERHOOK_H
#define USSERVICEEVENTLISTENERHOOK_H

#include <usServiceInterface.h>
#include <usServiceListenerHook.h>
#include <usShrinkableVector.h>
#include <usShrinkableMap.h>

namespace us {

class ModuleContext;
class ServiceEvent;

/**
 * @ingroup MicroServices
 *
 * Service Event Listener Hook Service.
 *
 * <p>
 * Modules registering this service will be called during service
 * (register, modify, and unregister service) operations.
 *
 * @remarks Implementations of this interface are required to be thread-safe.
 */
struct MITKCPPMICROSERVICES_EXPORT ServiceEventListenerHook
{
  typedef ShrinkableMap<ModuleContext*, ShrinkableVector<ServiceListenerHook::ListenerInfo> > ShrinkableMapType;

  virtual ~ServiceEventListenerHook();

  /**
   * Event listener hook method. This method is called prior to service event
   * delivery when a publishing module registers, modifies or unregisters a
   * service. This method can filter the listeners which receive the event.
   *
   * @param event The service event to be delivered.
   * @param listeners A map of Module Contexts to a list of Listener
   *        Infos for the module's listeners to which the specified event will
   *        be delivered. The implementation of this method may remove module
   *        contexts from the map and listener infos from the list
   *        values to prevent the event from being delivered to the associated
   *        listeners.
   */
  virtual void Event(const ServiceEvent& event, ShrinkableMapType& listeners) = 0;
};

}

#endif // USSERVICEEVENTLISTENERHOOK_H
