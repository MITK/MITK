/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkXML2EventParser_h
#define mitkXML2EventParser_h

#include <MitkCoreExports.h>

#include <mitkPropertyList.h>

#include <vtkXMLParser.h>

#include <mitkInteractionEvent.h>

namespace us
{
  class Module;
}

namespace mitk
{
  /**
     * \class XML2EventParser
     * \brief Generates a list of InteractionEvents by parsing an XML file.
     *
     * Reads an XML event configuration file and constructs a list of
     * InteractionEvent objects that can be used for interaction testing.
     *
     * \sa EventRecorder
     * \ingroup Interaction
     */
  class MITKCORE_EXPORT XML2EventParser : public vtkXMLParser
  {
  public:
    /**
     * \brief Construct an InteractionEventList object from an XML resource file.
     *
     * Uses the specified resource file containing an XML event configuration to
     * construct the event list. If the resource is invalid, no events are created.
     *
     * \param[in] filename  The resource name relative to the Interactions resource folder.
     * \param[in] module    The module from which to load the resource. If nullptr,
     *                      the current module context is used.
     */
    XML2EventParser(const std::string &filename, const us::Module *module = nullptr);

    /**
     * \brief Construct an InteractionEventList object from an input stream.
     *
     * Uses the specified input stream containing XML event configuration data to
     * construct the event list. If the stream contains invalid XML, no events are
     * created and an error is logged.
     *
     * \param[in] inputStream  Input stream referring to XML configuration data.
     */
    XML2EventParser(std::istream &inputStream);

    /** \brief Container type for the parsed interaction events. */
    typedef std::vector<mitk::InteractionEvent::Pointer> EventContainerType;

    /**
     * \brief Get the list of parsed interaction events.
     *
     * \return A copy of the container with all parsed InteractionEvent objects.
     */
    EventContainerType GetInteractions() { return m_InteractionList; }

    /** \brief Destructor. */
    ~XML2EventParser() override{};

  protected:
    /**
     * \brief Handle the start of an XML element during parsing.
     *
     * \param[in] elementName  Name of the XML element.
     * \param[in] atts         Null-terminated array of attribute name/value pairs.
     */
    void StartElement(const char *elementName, const char **atts) override;

    /**
     * \brief Handle the end of an XML element during parsing.
     *
     * When an event variant element ends, the collected properties are used to
     * create an InteractionEvent and add it to the list.
     *
     * \param[in] elementName  Name of the XML element.
     */
    void EndElement(const char *elementName) override;

    /**
     * \brief Read a string attribute value from the XML attribute array.
     *
     * \param[in] name  The attribute name to look for.
     * \param[in] atts  Null-terminated array of attribute name/value pairs.
     * \return The attribute value, or an empty string if not found.
     */
    std::string ReadXMLStringAttribute(const std::string &name, const char **atts);

    /**
     * \brief Read a boolean attribute value from the XML attribute array.
     *
     * The attribute is compared case-insensitively against "TRUE".
     *
     * \param[in] name  The attribute name to look for.
     * \param[in] atts  Null-terminated array of attribute name/value pairs.
     * \return True if the attribute value is "TRUE" (case-insensitive), false otherwise.
     */
    bool ReadXMLBooleanAttribute(const std::string &name, const char **atts);

  private:
    PropertyList::Pointer m_EventPropertyList;

    EventContainerType m_InteractionList;
  };

} // namespace mitk

#endif
