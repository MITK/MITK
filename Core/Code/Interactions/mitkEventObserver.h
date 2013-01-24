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
#include "mitkEventHandler.h"
#include "mitkInteractionEvent.h"
#include <MitkExports.h>

namespace mitk
{

  class MITK_CORE_EXPORT EventObserver : public EventHandler {

  public:
    mitkClassMacro(EventObserver,EventHandler);
    itkNewMacro(Self);
    /**
     * By this method all registered EventObersers are notified about every InteractionEvent,
     * the isHandled flag indicates if a DataInteractor has already handled that event.
     * EventObserver that trigger an action when observing an event may consider
     * this in order to not confuse the user by, triggering several independant action with one
     * single user event (such as a mouse click)
     */
    virtual void Notify(InteractionEvent::Pointer interactionEvent,bool isHandled);

  protected:
    EventObserver();
    virtual ~EventObserver();
  protected:


  private:

  };

} /* namespace mitk */

#endif /* EventObserver_h */
