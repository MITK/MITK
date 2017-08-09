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

#ifndef mitkStateMachineConfig_h
#define mitkStateMachineConfig_h

#include <MitkCoreExports.h>

#include "mitkPropertyList.h"
#include "usSharedData.h"

#include "itkSmartPointer.h"

namespace us
{
  class Module;
}

namespace mitk
{
  class InteractionEvent;
  struct EventConfigPrivate;

  /**
   * \class EventConfig
   * \brief Configuration Object for Statemachines.
   *
   * Reads given config file, which translates specific user inputs (InteractionEvents) into EventVariants that can be
   *processed
   * by the StateMachine.
   * Refer to \ref ConfigFileDescriptionSection .
   *
   * @ingroup Interaction
   **/
  class MITKCORE_EXPORT EventConfig
  {
  public:
    typedef itk::SmartPointer<InteractionEvent> EventType;

    /**
     * @brief Constructs an invalid EventConfig object.
     *
     * Call LoadConfig to create a valid configuration object.
     */
    EventConfig();
    EventConfig(const EventConfig &other);

    /**
     * @brief Construct an EventConfig object based on a XML configuration file.
     *
     * Uses the specified resource file containing an XML event configuration to
     * construct an EventConfig object. If the resource is invalid, the created
     * EventConfig object will also be invalid.
     *
     * @param filename The resource name relative to the Interactions resource folder.
     * @param module
     */
    EventConfig(const std::string &filename, const us::Module *module = nullptr);

    /**
     * @brief Construct an EventConfig object based on a XML configuration file.
     *
     * Uses the specified istream refering to a file containing an XML event configuration to
     * construct an EventConfig object. If the resource is invalid, the created
     * EventConfig object will also be invalid.
     *
     * @param inputStream std::ifstream to XML configuration file
     */
    EventConfig(std::istream &inputStream);

    /**
     * @brief Construct an EventConfig object based on a vector of mitk::PropertyLists
     *
     * Constructs the EventObject based on a description provided by vector of property values, where each
     mitk::PropertyList describes
     * one Event.
     * <b> Example </b>
     \code
      #include "mitkEventConfig.h"
      #include "mitkInteractionEventConst.h"
      #include "mitkPropertyList.h"

      // First event
      mitk::PropertyList::Pointer propertyList1 = mitk::PropertyList::New();
      // Setting the EventClass property to 'MousePressEvent'
      propertyList1->SetStringProperty(mitk::InteractionEventConst::xmlParameterEventClass.c_str(), "MousePressEvent");
      // Setting the Event variant value to 'MousePressEventVariantÄ
      propertyList1->SetStringProperty(mitk::InteractionEventConst::xmlParameterEventVariant.c_str(),
     "MousePressEventVariant");
      // set control and alt buttons as modifiers
      propertyList1->SetStringProperty("Modifiers","CTRL,ALT");
      // Second event
      mitk::PropertyList::Pointer propertyList2 = mitk::PropertyList::New();
      propertyList2->SetStringProperty(mitk::InteractionEventConst::xmlParameterEventClass.c_str(),
     "MouseReleaseEvent");
      propertyList2->SetStringProperty(mitk::InteractionEventConst::xmlParameterEventVariant.c_str(),
     "MouseReleaseEventVariant");
      propertyList2->SetStringProperty("Modifiers","SHIFT");

      // putting both descriptions in a vector
      std::vector<mitk::PropertyList::Pointer>* configDescription = new std::vector<mitk::PropertyList::Pointer>();
      configDescription->push_back(propertyList1);
      configDescription->push_back(propertyList2);
      // create the config object
      mitk::EventConfig newConfig(configDescription);
     \endcode
     */
    EventConfig(const std::vector<PropertyList::Pointer> &configDescription);

    EventConfig &operator=(const EventConfig &other);

    ~EventConfig();

    /**
     * @brief Checks wether this EventConfig object is valid.
     * @return Returns \c true if a configuration was successfully loaded, \c false otherwise.
     */
    bool IsValid() const;

    /**
     * @brief This method \e extends this configuration.
     *
     * The configuration from the resource provided is loaded and only the ones conflicting are replaced by the new one.
     * This way several configuration files can be combined.
     *
     * @see AddConfig(const EventConfig&)
     * @see InteractionEventHandler::AddEventConfig(const std::string&, const Module*)
     *
     * @param filename The resource name relative to the Interactions resource folder.
     * @param module The module containing the resource. Defaults to the Mitk module.
     * @return \c true if the configuration was successfully added, \c false otherwise.
     */
    bool AddConfig(const std::string &filename, const us::Module *module = nullptr);

    /**
     * @brief This method \e extends this configuration.
     * The configuration from the EventConfig object is loaded and only the ones conflicting are replaced by the new
     * one.
     * This way several configurations can be combined.
     *
     * @see AddConfig(const std::string&, const Module*)
     * @see InteractionEventHandler::AddEventConfig(const EventConfig&)
     *
     * @param config The EventConfig object whose configuration should be added.
     * @return \c true if the configuration was successfully added, \c false otherwise.
     */
    bool AddConfig(const EventConfig &config);

    /**
     * @brief Reset this EventConfig object, rendering it invalid.
     */
    void ClearConfig();

    /**
     * Returns a PropertyList that contains the properties set in the configuration file.
     * All properties are stored as strings.
     */
    PropertyList::Pointer GetAttributes() const;

    /**
     * Checks if the config object has a definition for the given event. If it has, the corresponding variant name is
     * returned, else
     * an empty string is returned.
     * \note mitk::InternalEvent is handled differently. Their signal name is returned as event variant. So there is no
     * need
     * to configure them in a config file.
     * \note mitk::InteractionKeyEvent may have a defined event variant, if this is the case, this function returns it.
     * If
     * no
     * such definition is found key events are mapped to Std + Key , so an 'A' will be return as 'StdA' .
     */
    std::string GetMappedEvent(const EventType &interactionEvent) const;

  private:
    us::SharedDataPointer<EventConfigPrivate> d;
  };

} // namespace mitk

#endif /* mitkStateMachineConfig_h */
