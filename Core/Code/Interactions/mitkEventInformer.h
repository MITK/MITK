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

#ifndef mitkEventInformer_h
#define mitkEventInformer_h

#include "mitkEventObserver.h"
#include "mitkInteractionEvent.h"
#include <usServiceInterface.h>


namespace mitk
{
  struct EventInformerService
  {
    virtual ~EventInformerService()
    {
    }

    virtual void RegisterObserver(EventObserver::Pointer eventObserver) = 0;
    virtual void UnRegisterObserver(EventObserver::Pointer eventObserver) = 0;
    virtual void NotifyObservers(InteractionEvent::Pointer interactionEvent) = 0;
  };

} /* namespace mitk */
US_DECLARE_SERVICE_INTERFACE(mitk::EventInformerService, "EventInformerService/1.0")
#endif
