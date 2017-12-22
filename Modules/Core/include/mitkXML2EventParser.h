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

#ifndef mitkXML2EventParser_h
#define mitkXML2EventParser_h

#include <MitkCoreExports.h>

#include "mitkPropertyList.h"

#include <vtkXMLParser.h>

#include "mitkInteractionEvent.h"

namespace us
{
  class Module;
}

namespace mitk
{
  /**
     * \class InteractionEventList
     * \brief Generates a list of InteractionEvents based on an XML file-
     *
     * @sa EventRecorder
     * @ingroup Interaction
     **/
  class MITKCORE_EXPORT XML2EventParser : public vtkXMLParser
  {
  public:
    /**
       * @brief Construct an InteractionEventList object based on a XML configuration file.
       *
       * Uses the specified resource file containing an XML event configuration to
       * construct an EventConfig object. If the resource is invalid, the created
       * EventConfig object will also be invalid.
       *
       * @param filename The resource name relative to the Interactions resource folder.
       * @param module
       */
    XML2EventParser(const std::string &filename, const us::Module *module = nullptr);

    /**
       * @brief Construct an InteractionEventList object based on a XML configuration file.
       *
       * Uses the specified istream refering to a file containing an XML event configuration to
       * construct an EventConfig object. If the resource is invalid, the created
       * EventConfig object will also be invalid.
       *
       * @param inputStream std::ifstream to XML configuration file
       */
    XML2EventParser(std::istream &inputStream);

    typedef std::vector<mitk::InteractionEvent::Pointer> EventContainerType;

    EventContainerType GetInteractions() { return m_InteractionList; }
    ~XML2EventParser() override{};

  protected:
    /**
       * @brief Derived from XMLReader
       **/
    void StartElement(const char *elementName, const char **atts) override;

    /**
       * @brief Derived from XMLReader
       **/
    void EndElement(const char *elementName) override;

    std::string ReadXMLStringAttribute(const std::string &name, const char **atts);
    bool ReadXMLBooleanAttribute(const std::string &name, const char **atts);

  private:
    PropertyList::Pointer m_EventPropertyList;

    EventContainerType m_InteractionList;
  };

} // namespace mitk

#endif /* mitkStateMachineConfig_h */
