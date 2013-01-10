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
    virtual void Notify(InteractionEvent::Pointer interactionEvent);

  protected:
    EventObserver();
    virtual ~EventObserver();
  protected:


  private:

  };

} /* namespace mitk */

#endif /* EventObserver_h */
