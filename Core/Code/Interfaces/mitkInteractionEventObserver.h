/*===================================================================

 The Medical Imaging Interaction Toolkit (MITK)

 Copyright (c) German Cancer Research Center,
 Division of Medical and Biological Informatics.
 All rights reserved.

 This software is distributed WITHOUT ANY WARRANTY; without
 even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE.

 See LICENSE.txt or http://www.mitk.org for details.

 ===================================================================*/

#ifndef InteractionEventObserver_h
#define InteractionEventObserver_h

#include <MitkCoreExports.h>
#include "mitkServiceInterface.h"
#include "mitkInteractionEvent.h"

namespace mitk
{
 /**
  * \class InteractionEventObserver
  * \brief Base class to implement InteractionEventObservers.
  *
  * This class also provides state machine infrastructure,
  * but usage thereof is optional. See the Notify method for more information.
  */

  struct MITK_CORE_EXPORT InteractionEventObserver
  {
    InteractionEventObserver();
    virtual ~InteractionEventObserver();
      /**
     * By this method all registered EventObersers are notified about every InteractionEvent,
     * the isHandled flag indicates if a DataInteractor has already handled that event.
     * InteractionEventObserver that trigger an action when observing an event may consider
     * this in order to not confuse the user by, triggering several independent action with one
     * single user event (such as a mouse click)
     *
     * If you want to use the InteractionEventObserver as a state machine give the event to the state machine by implementing, e.g.
     \code
     void mitk::InteractionEventObserver::Notify(InteractionEvent::Pointer interactionEvent, bool isHandled)
      {
        if (!isHandled) {
          this->HandleEvent(interactionEvent, NULL);
        }
      }
      \endcode

     * This overwrites the FilterEvents function of the EventStateMachine to ignore the DataNode, since InteractionEventObservers are not associated with one.
    virtual bool FilterEvents(InteractionEvent* interactionEvent, DataNode* dataNode);
     */
    virtual void Notify(InteractionEvent* interactionEvent,bool isHandled) = 0;

    void Disable();
    void Enable();
    bool IsEnabled() const;
  private:
    bool m_IsEnabled;
  };

} /* namespace mitk */
MITK_DECLARE_SERVICE_INTERFACE(mitk::InteractionEventObserver, "org.mitk.InteractionEventObserver")
#endif /* InteractionEventObserver_h */
