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

#ifndef mitkEventFactory_h
#define mitkEventFactory_h

#include "mitkCommon.h"
#include "mitkInteractionEvent.h"
#include "mitkPropertyList.h"
#include <MitkExports.h>


namespace mitk
{
/**
 * \class EventFactory
 * \brief Generates InteractionEvent-Objects/XML.
 *
 * 1) InteractionEvents can be constructed by a PropertyList describing the event (see mitk::EventConfig for examples)
 * 2) An XML description of InteractionEvents can be retrieved.
 *
 * This class is used by the EventConfig object to parse configuration files and create Events based on the xml description.
 *
 * \ingroup Interaction
 */
  class MITK_CORE_EXPORT EventFactory
  {
  public:


    /**
     * Parses PropertyList and queries all possible Information.
     * If an attribute is not present the default value is used.
     * Finally the Event-Type is chosen by the ClassName property and the object is created using the collected information.
     */
    static InteractionEvent::Pointer CreateEvent(PropertyList::Pointer eventDescription);

    /**
     * @brief EventToXML Transforms an event into a XML tag describing it.
     * @param event
     * @return Event block specifying event class and attributes of that event.
     */
    static std::string EventToXML(InteractionEvent* event);

  private:
    /**
     * @brief GetEventButton Return EventButton as String
     * @param event
     * @return
     */
    static std::string GetButtonState(InteractionEvent* event);

    /**
     * @brief GetModifierState Return ModifierState as String
     * @param event
     * @return
     */
    static std::string GetModifierState(InteractionEvent* event);

    /**
     * @brief GetEventButton Return EventButton as String
     * @param event
     * @return
     */
    static std::string GetEventButton(InteractionEvent* event);

    /**
     * @brief GetPosition Return World Position as String
     * @param event
     * @return
     */
    static std::string GetPositionInWorld(InteractionEvent* event);

    /**
     * @brief GetPositionOnScreen Return PositionOnScreen as String
     * @param event
     * @return
     */
    static std::string GetPositionOnScreen(InteractionEvent* event);

  };
}

#endif /* mitkEventFactory_h */
