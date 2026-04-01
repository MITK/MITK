/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkEventInformer_h
#define mitkEventInformer_h

#include <mitkInteractionEvent.h>
#include <mitkInteractionEventObserver.h>
#include <mitkServiceInterface.h>

namespace mitk
{
  /**
   * \brief Interface for the micro service that manages InteractionEventObserver registration and notification.
   *
   * This service takes care of informing registered InteractionEventObserver instances about new events.
   *
   * \ingroup Interaction
   *
   * \sa InteractionEventObserver
   * \sa InteractionEvent
   */
  struct EventInformerService
  {
    virtual ~EventInformerService() {}

    /**
     * \brief Register an observer to receive interaction event notifications.
     *
     * \param InteractionEventObserver The observer to register.
     */
    virtual void RegisterObserver(InteractionEventObserver::Pointer InteractionEventObserver) = 0;

    /**
     * \brief Unregister a previously registered observer.
     *
     * \param InteractionEventObserver The observer to unregister.
     */
    virtual void UnRegisterObserver(InteractionEventObserver::Pointer InteractionEventObserver) = 0;

    /**
     * \brief Notify all registered observers about an interaction event.
     *
     * \param interactionEvent The event to broadcast to all observers.
     */
    virtual void NotifyObservers(InteractionEvent::Pointer interactionEvent) = 0;
  };

} /* namespace mitk */
MITK_DECLARE_SERVICE_INTERFACE(mitk::EventInformerService, "EventInformerService/1.0")
#endif
