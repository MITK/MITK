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

#include <vtkXMLParser.h>
#include <iostream>
#include "itkObject.h"

#include "itkObjectFactory.h"
#include "mitkCommon.h"
#include <MitkExports.h>
#include <mitkPropertyList.h>
#include <string>

namespace mitk
{

  class InteractionEvent;

  /**
   *@brief Configuration Object for Statemachines.
   *
   * Reads given config file, which translates specific user inputs (InteractionEvents) into EventVariants that can be processed
   * by the StateMachine.
   * @ingroup Interaction
   **/

  class MITK_CORE_EXPORT EventConfig: public vtkXMLParser
  {
  public:

    static EventConfig *New();
    vtkTypeMacro(EventConfig,vtkXMLParser);

    typedef itk::SmartPointer<InteractionEvent> EventType;

    /**
    * @brief Loads XML resource
    *
    * Loads a XML resource file in the given module context.
    * The files have to be placed in the Resources/Interaction folder of their respective module.
    **/
    bool LoadConfig(std::string fileName, std::string moduleName = "Mitk");

    void ClearConfig();
    /**
     * Returns a PropertyList that contains the properties set in the configuration file.
     * All properties are stored as strings.
     */
    const PropertyList::Pointer GetPropertyList();

    /**
     * Checks if the config object has a definition for the given event. If it has the corresponding variant name is return, else
     * an empty string is returned.
     * \note mitk::InternalEvents are handled differently. Their signal name is returned as event variant. So there is no need
     * to configure them in a config file.
     * \note mitk::InteractionKeys my have a defined event variant, if this is the case, this function returns it. If no
     * such definition is found key events are mapped to Std + Key , so an 'A' will be return as 'StdA' .
     */
    std::string GetMappedEvent(InteractionEvent* interactionEvent);

  protected:

    EventConfig();
    virtual ~EventConfig();

    /**
     * @brief Derived from XMLReader
     **/
    void StartElement(const char* elementName, const char **atts);
    /**
     * @brief Derived from XMLReader
     **/
    void EndElement(const char* elementName);

  private:
    /**
     * @brief Derived from XMLReader
     **/
    std::string ReadXMLStringAttribut(std::string name, const char** atts);
    /**
     * @brief Derived from XMLReader
     **/
    bool ReadXMLBooleanAttribut(std::string name, const char** atts);


    /**
     * @brief List of all global properties of the config object.
     */
    PropertyList::Pointer m_PropertyList;

    /**
     * @brief Temporal list of all properties of a Event. Used to parse an Input-Event and collect all parameters between the two <input>
     * and </input> tags.
     */
    PropertyList::Pointer m_EventPropertyList;

    struct EventMapping
    {
      std::string variantName;
      EventType interactionEvent;
    };
    /**
     * Checks if mapping with the same parameters already exists, if so, it is replaced,
     * else the new mapping added
     */
    void InsertMapping(EventMapping mapping);

    typedef std::list<EventMapping> EventListType;
    EventMapping m_CurrEventMapping;

    /**
     * Stores InteractionEvents and their corresponding VariantName
     */
    EventListType m_EventList;
    bool m_Errors; // use member, because of inheritance from vtkXMLParser we can't return a success value for parsing the file.
  };

} // namespace mitk

#endif /* mitkStateMachineConfig_h */
