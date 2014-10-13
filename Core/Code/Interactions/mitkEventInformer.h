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

#include "mitkInteractionEventObserver.h"
#include "mitkInteractionEvent.h"
#include <mitkServiceInterface.h>


namespace mitk
{
  /**
   * Serves as an interface for the micro service that takes care of informing/registering InteractionEventObserver
   */
  struct EventInformerService
  {
    virtual ~EventInformerService()
    {
    }

    virtual void RegisterObserver(InteractionEventObserver::Pointer InteractionEventObserver) = 0;
    virtual void UnRegisterObserver(InteractionEventObserver::Pointer InteractionEventObserver) = 0;
    virtual void NotifyObservers(InteractionEvent::Pointer interactionEvent) = 0;
  };

} /* namespace mitk */
MITK_DECLARE_SERVICE_INTERFACE(mitk::EventInformerService, "EventInformerService/1.0")
#endif
