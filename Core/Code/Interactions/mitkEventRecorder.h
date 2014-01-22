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

#ifndef mitkEventRecorder_h
#define mitkEventRecorder_h

#include <MitkExports.h>
#include "mitkInteractionEventObserver.h"

namespace mitk
{
  /**
   *\class EventRecorder
   *@brief Observer that enables recoding of all user interaction with the render windows and storing it in an XML file.
   *
   * @ingroup Interaction
   **/
  class MITK_CORE_EXPORT EventRecorder: public InteractionEventObserver
  {
  public:
    EventRecorder(){}
    ~EventRecorder(){}

    /**
     * By this function the Observer gets notified about new events.
     * Here it is adapted to pass the events to the state machine in order to use
     * its infrastructure.
     * It also checks if event is to be accepted when it already has been processed by a DataInteractor.
     */
    virtual void Notify(InteractionEvent* interactionEvent, bool);

  private:

  };
}
#endif
