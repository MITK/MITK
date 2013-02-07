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

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "mitkEvent.h"
#include "mitkCommon.h"
#include <MitkExports.h>
#include "mitkEventConfig.h"
#include "mitkPropertyList.h"
#include <string>


namespace mitk
{
  /**
   * \class EventHandler
   *  Base Class for Interaction handling classes.
   *  Handles loading of configuration object and mapping of events to variant names.
   */
  class InteractionEvent;
  class MITK_CORE_EXPORT EventHandler : public itk::Object {

  public:
    mitkClassMacro(EventHandler, itk::Object);
    itkNewMacro(Self);
    /**
    * @brief Loads XML resource
    *
    * Loads a XML resource file in the given module context.
    * Default is the Mitk module (core).
    * The files have to be placed in the Resources/Interaction folder of their respective module.
    * This method will remove all existing configuration and replaces it with the new one.
    */
    virtual bool LoadEventConfig(std::string filename, std::string moduleName = "Mitk");
    /**
     * This method EXTENDs the configuration.
     * The configuration from the resource provided is loaded and only the ones conflicting are replaced by the new one.
     * This way several configuration files can be combined
     */
    virtual bool AddEventConfig(std::string filename, std::string moduleName = "Mitk");

    /**
     * Returns a PropertyList in which the parameters defined in the config file are listed.
     */
    PropertyList::Pointer GetPropertyList();

  protected:
    EventHandler();
    virtual ~EventHandler();
    std::string GetMappedEvent(InteractionEvent* interactionEvent);

    /**
     * Is called whenever a new config object ist set.
     * Overwrite this method e.g. to initialize EventHandler with parameters in configuration file.
     */
    virtual void ConfigurationChanged();

  private:
    EventConfig* m_EventConfig;

  };

} /* namespace mitk */
#endif /* MITKEVENTHANDLER_H_ */
