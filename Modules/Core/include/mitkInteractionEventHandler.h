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

#ifndef MITKEVENTHANDLER_H_
#define MITKEVENTHANDLER_H_

#include "itkLightObject.h"
#include "itkObjectFactory.h"

#include "mitkCommon.h"
#include "mitkEventConfig.h"
#include "mitkPropertyList.h"
#include <MitkCoreExports.h>

#include <string>

namespace us
{
  class Module;
}

namespace mitk
{
  /**
   * \class EventHandler
   *  Serves as a base class for all objects and classes that handle mitk::InteractionEvents.
   *
   *  It provides an interface to load configuration objects map of events to variant names.
   */
  class InteractionEvent;
  class MITKCORE_EXPORT InteractionEventHandler : public itk::Object
  {
  public:
    mitkClassMacroItkParent(InteractionEventHandler, itk::Object)

      /**
       * @brief Loads a configuration from an XML resource.
       *
       * Loads an event configuration from an XML resource file contained in the given module.
       * Default is the Mitk module (core).
       * The files have to be placed in the Resources/Interactions folder of their respective module.
       * This method will remove all existing configuration and replaces it with the new one.
       *
       * @see SetEventConfig(const EventConfig&)
       *
       * @param filename The resource name relative to the Interactions resource folder.
       * @param module The module containing the resource. Defaults to the Mitk module.
       * @return \c true if the resource was successfully loaded, \c false otherwise.
       */
      bool SetEventConfig(const std::string &filename, const us::Module *module = nullptr);

    /**
     * @brief Loads a configuration from an EventConfig object.
     *
     * Loads an event configuration from the given EventConfig object. This method will remove
     * all existing configuration and replaces it with the new one.
     *
     * @see SetEventConfig(const std::string&, const Module*)
     *
     * @param config The EventConfig object containing the new configuration.
     * @return \c true if the configuration was successfully loaded, \c false otherwise.
     */
    bool SetEventConfig(const EventConfig &config);

    /**
     * @brief Returns the current configuration.
     * @return A EventConfig object representing the current event configuration.
     */
    EventConfig GetEventConfig() const;

    /**
     * @brief This method \e extends the configuration.
     *
     * The configuration from the resource provided is loaded and only the ones conflicting are replaced by the new one.
     * This way several configuration files can be combined.
     *
     * @see AddEventConfig(const EventConfig&)
     *
     * @param filename The resource name relative to the Interactions resource folder.
     * @param module The module containing the resource. Defaults to the Mitk module.
     * @return \c true if the configuration was successfully added, \c false otherwise.
     */
    bool AddEventConfig(const std::string &filename, const us::Module *module = nullptr);

    /**
     * @brief This method \e extends the configuration.
     * The configuration from the EventConfig object is loaded and only the ones conflicting are replaced by the new
     * one.
     * This way several configurations can be combined.
     *
     * @see AddEventConfig(const std::string&, const Module*)
     *
     * @param config The EventConfig object whose configuration should be added.
     * @return \c true if the configuration was successfully added, \c false otherwise.
     */
    bool AddEventConfig(const EventConfig &config);

  protected:
    InteractionEventHandler();
    ~InteractionEventHandler() override;

    /**
     * Returns a PropertyList in which the parameters defined in the config file are listed.
     */
    PropertyList::Pointer GetAttributes() const;

    std::string MapToEventVariant(InteractionEvent *interactionEvent);

    /**
     * Is called whenever a new config object ist set.
     * Overwrite this method e.g. to initialize EventHandler with parameters in configuration file.
     */
    virtual void ConfigurationChanged();

  private:
    EventConfig m_EventConfig;
  };

} /* namespace mitk */
#endif /* MITKEVENTHANDLER_H_ */
