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
#include "mitkInteractionEventObserver.h"
#include "itkObject.h"
#include "itkObjectFactory.h"
#include <list>
#include "mitkCommon.h"

namespace mitk
{
  /**
   * \class InformerService
   *
   * MicroService that handles informing the InteractionEventObservers about InteractionEvents.
   * Also provides an interface to register and unregister InteractionEventObservers for InteractionEvents
   */
  class MITK_CORE_EXPORT InformerService: public itk::Object
  {
  public:
    /**
     * \brief Register a new InteractionEventObserver
     */
    void RegisterObserver(InteractionEventObserver* InteractionEventObserver);
    /**
     * \brief Unregister an InteractionEventObserver
     */
    void UnRegisterObserver(InteractionEventObserver* InteractionEventObserver);
    /**
     * \brief Distributes an event to all registered observers
     */
    void NotifyObservers(InteractionEvent::Pointer interactionEvent, bool isHandled);

    InformerService();
    ~InformerService();

  private:
    std::list<InteractionEventObserver::Pointer> m_ListObserver; // list that stores all registered observers
  };

} /* namespace mitk */
US_DECLARE_SERVICE_INTERFACE(mitk::InformerService, "org.mitk.InformerService")
#endif
