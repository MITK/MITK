/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkInteractionEventObserver_h
#define mitkInteractionEventObserver_h

#include <mitkInteractionEvent.h>
#include <mitkServiceInterface.h>
#include <MitkCoreExports.h>

namespace mitk
{
  /**
   * \ingroup Interaction
   * \ingroup MicroServices_Interfaces
   *
   * \class InteractionEventObserver
   * \brief Interface for observing all InteractionEvents dispatched through the system.
   *
   * Classes implementing this interface are notified about every InteractionEvent
   * after it has been offered to all DataInteractors. Observers are registered as
   * micro services and can optionally be scoped to a specific renderer.
   *
   * This class also provides optional state machine infrastructure.
   * See the Notify() method documentation for details on how to use it
   * with or without a state machine.
   *
   * \sa Dispatcher
   * \sa EventStateMachine
   */

  struct MITKCORE_EXPORT InteractionEventObserver
  {
    InteractionEventObserver();
    virtual ~InteractionEventObserver();

    /**
     * \brief Called by the Dispatcher to notify this observer about an InteractionEvent.
     *
     * All registered InteractionEventObservers receive every event after it has been
     * offered to DataInteractors. The \a isHandled flag indicates whether a DataInteractor
     * has already handled the event. Observers that trigger actions should consider this
     * flag to avoid confusing the user with multiple independent actions from one event.
     *
     * To use state machine infrastructure, forward the event to HandleEvent():
     * \code
     * void MyObserver::Notify(InteractionEvent::Pointer interactionEvent, bool isHandled)
     * {
     *   if (!isHandled) {
     *     this->HandleEvent(interactionEvent, nullptr);
     *   }
     * }
     * \endcode
     *
     * \param[in] interactionEvent The event being dispatched.
     * \param[in] isHandled True if a DataInteractor already handled this event.
     */
    virtual void Notify(InteractionEvent *interactionEvent, bool isHandled) = 0;

    /** \brief Disable this observer so it no longer receives notifications. */
    void Disable();

    /** \brief Enable this observer so it receives notifications. */
    void Enable();

    /**
     * \brief Check whether this observer is currently enabled.
     * \return True if enabled, false if disabled.
     */
    bool IsEnabled() const;

  private:
    bool m_IsEnabled;
  };

} /* namespace mitk */
MITK_DECLARE_SERVICE_INTERFACE(mitk::InteractionEventObserver, "org.mitk.InteractionEventObserver")
#endif
