/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkEventFactory_h
#define mitkEventFactory_h

#include <mitkCommon.h>
#include <mitkInteractionEvent.h>
#include <mitkPropertyList.h>
#include <MitkCoreExports.h>

namespace mitk
{
  /**
   * \class EventFactory
   * \brief Generates InteractionEvent-Objects/XML.
   *
   * 1) InteractionEvents can be constructed by a PropertyList describing the event (see mitk::EventConfig for examples)
   * 2) An XML description of InteractionEvents can be retrieved.
   *
   * This class is used by the EventConfig object to parse configuration files and create Events based on the xml
   * description.
   *
   * \ingroup Interaction
   */
  class MITKCORE_EXPORT EventFactory
  {
  public:
    /**
     * \brief Create an InteractionEvent from a property list description.
     *
     * Parses the PropertyList and queries all possible information.
     * If an attribute is not present the default value is used.
     * Finally the Event-Type is chosen by the ClassName property and the object is created using the collected
     * information.
     *
     * \param eventDescription A PropertyList describing the event attributes.
     * \return A newly created InteractionEvent, or nullptr if creation failed.
     *
     * \sa EventConfig
     */
    static InteractionEvent::Pointer CreateEvent(PropertyList::Pointer eventDescription);

    /**
     * \brief Transform an event into an XML tag describing it.
     *
     * \param event The InteractionEvent to serialize.
     * \return XML string block specifying the event class and attributes.
     */
    static std::string EventToXML(InteractionEvent *event);
  };
}

#endif
