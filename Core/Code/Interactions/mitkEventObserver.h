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

#ifndef EventObserver_h
#define EventObserver_h

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "mitkCommon.h"
#include "mitkEventStateMachine.h"
#include "mitkInteractionEvent.h"
#include <MitkExports.h>

namespace mitk
{
 /**
  * \class EventObserver
  * \brief Base class to implement EventObservers.
  *
  * This class also provides state machine infrastructure,
  * but usage thereof is optional. See the Notify method for more information.
  */

  class MITK_CORE_EXPORT EventObserver : public EventStateMachine {

  public:
    mitkClassMacro(EventObserver,EventHandler);
    itkNewMacro(Self);
    /**
     * By this method all registered EventObersers are notified about every InteractionEvent,
     * the isHandled flag indicates if a DataInteractor has already handled that event.
     * EventObserver that trigger an action when observing an event may consider
     * this in order to not confuse the user by, triggering several independent action with one
     * single user event (such as a mouse click)
     *
     * If you want to use the EventObserver as a state machine give the event to the state machine by implementing, e.g.
     \code
     void mitk::EventObserver::Notify(InteractionEvent::Pointer interactionEvent, bool isHandled)
      {
        if (!isHandled) {
          this->HandleEvent(interactionEvent, NULL);
        }
      }
      \endcode
     */
    virtual void Notify(InteractionEvent::Pointer interactionEvent,bool isHandled);


  protected:
    EventObserver();
    virtual ~EventObserver();

    /**
     * Overwrite this to ignore the DataNode, since EventObservers are not associated with one.
     */
    virtual bool FilterEvents(InteractionEvent* interactionEvent, DataNode* dataNode);
  protected:


  private:

  };

} /* namespace mitk */

#endif /* EventObserver_h */
