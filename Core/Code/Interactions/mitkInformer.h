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

#ifndef mitkInformerService_h
#define mitkInformerService_h

#include "mitkEventInformer.h"
#include "mitkInteractionEvent.h"
#include "mitkEventObserver.h"
#include "itkObject.h"
#include "itkObjectFactory.h"
#include <list>
#include "mitkCommon.h"
#include <mitkModuleActivator.h>
#include <mitkModuleContext.h>

namespace mitk
{
  class MITK_CORE_EXPORT InformerService: public itk::Object
  {
  public:
    void RegisterObserver(EventObserver* eventObserver);
    void UnRegisterObserver(EventObserver* eventObserver);
    void NotifyObservers(InteractionEvent::Pointer interactionEvent, bool isHandled);

    InformerService();
    ~InformerService();

  private:
    std::list<EventObserver::Pointer> m_ListObserver;
  };

} /* namespace mitk */
US_DECLARE_SERVICE_INTERFACE(mitk::InformerService, "org.mitk.InformerService")
#endif
