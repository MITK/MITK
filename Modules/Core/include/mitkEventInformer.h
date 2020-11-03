/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkEventInformer_h
#define mitkEventInformer_h

#include "mitkInteractionEvent.h"
#include "mitkInteractionEventObserver.h"
#include <mitkServiceInterface.h>

namespace mitk
{
  /**
   * Serves as an interface for the micro service that takes care of informing/registering InteractionEventObserver
   */
  struct EventInformerService
  {
    virtual ~EventInformerService() {}
    virtual void RegisterObserver(InteractionEventObserver::Pointer InteractionEventObserver) = 0;
    virtual void UnRegisterObserver(InteractionEventObserver::Pointer InteractionEventObserver) = 0;
    virtual void NotifyObservers(InteractionEvent::Pointer interactionEvent) = 0;
  };

} /* namespace mitk */
MITK_DECLARE_SERVICE_INTERFACE(mitk::EventInformerService, "EventInformerService/1.0")
#endif
