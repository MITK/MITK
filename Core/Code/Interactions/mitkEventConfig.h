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

#include <MitkExports.h>

#include "mitkSharedData.h"
#include "mitkPropertyList.h"

#include "itkSmartPointer.h"

namespace mitk
{

  class InteractionEvent;
  struct EventConfigPrivate;

  /**
   * \class EventConfig
   * \brief Configuration Object for Statemachines.
   *
   * Reads given config file, which translates specific user inputs (InteractionEvents) into EventVariants that can be processed
   * by the StateMachine.
   * Refer to \ref ConfigFileDescriptionSection .
   *
   * @ingroup Interaction
   **/
  class MITK_CORE_EXPORT EventConfig
  {

  public:

    typedef itk::SmartPointer<InteractionEvent> EventType;

    /**
     * @brief Constructs an invalid EventConfig object.
     *
     * Call LoadConfig to create a valid configuration object.
     */
    EventConfig();
    EventConfig(const EventConfig& other);

    EventConfig& operator=(const EventConfig& other);

    ~EventConfig();

    /**
     * @brief Checks wether this EventConfig object is valid.
     * @return Returns \c true if a configuration was successfully loaded, \c false otherwise.
     */
    bool IsValid() const;

    /**
     * @brief Loads XML resource
     *
     * Loads a XML resource file in the given module context.
     * The files have to be placed in the Resources/Interaction folder of their respective module.
     **/
    bool LoadConfig(const std::string& fileName, const std::string& moduleName = "Mitk");

    void ClearConfig();

    /**
     * Returns a PropertyList that contains the properties set in the configuration file.
     * All properties are stored as strings.
     */
    PropertyList::Pointer GetAttributes() const;

    /**
     * Checks if the config object has a definition for the given event. If it has, the corresponding variant name is returned, else
     * an empty string is returned.
     * \note mitk::InternalEvents are handled differently. Their signal name is returned as event variant. So there is no need
     * to configure them in a config file.
     * \note mitk::InteractionKeys may have a defined event variant, if this is the case, this function returns it. If no
     * such definition is found key events are mapped to Std + Key , so an 'A' will be return as 'StdA' .
     */
    std::string GetMappedEvent(const EventType& interactionEvent) const;

  private:

    SharedDataPointer<EventConfigPrivate> d;

  };

} // namespace mitk

#endif /* mitkStateMachineConfig_h */
