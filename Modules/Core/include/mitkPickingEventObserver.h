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

#ifndef PickingEventObserver_h
#define PickingEventObserver_h

#include <MitkCoreExports.h>
#include "mitkInteractionEventObserver.h"

namespace mitk
{
    /**
    * \class PickingEventObserver
    */
  class MITKCORE_EXPORT PickingEventObserver : public InteractionEventObserver
    {
    public:
        PickingEventObserver();
        virtual ~PickingEventObserver();

        /**
        * By this function the Observer gets notified about new events.
        */
        virtual void Notify(InteractionEvent* interactionEvent, bool isHandled);

    protected:
        // Handle the event that is used for picking - a mouse release event with no dragging
        // Default implementation dispatches the control to
        // - HandlePickOneEvent() if no modifier keys are pressed
        // - HandlePickAddEvent() if shift key is pressed
        // - HandlePickToggleEvent() if control key is pressed
        virtual void HandlePickingEvent(InteractionEvent* interactionEvent);

        virtual void HandlePickOneEvent(InteractionEvent* interactionEvent) = 0;
        virtual void HandlePickAddEvent(InteractionEvent* interactionEvent) = 0;
        virtual void HandlePickToggleEvent(InteractionEvent* interactionEvent) = 0;
    private:
        bool m_startedDragging;
        bool m_isDragging;
    };

} /* namespace mitk */

#endif /* PickingEventObserver_h */
