/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPropertyListsXmlFileReaderAndWriter_h
#define mitkPropertyListsXmlFileReaderAndWriter_h

#include <mitkDataStorage.h>

namespace tinyxml2
{
  class XMLElement;
}

namespace mitk
{
  /**
   * \brief Reads and writes collections of named PropertyList objects to and from XML files.
   *
   * This class serializes PropertyList maps (keyed by string identifiers) into a simple
   * XML format and deserializes them back. Each PropertyList is stored as a
   * \c \<PropertyList\> element containing \c \<Property\> child elements. Supported property
   * types are BoolProperty, StringProperty, IntProperty, DoubleProperty, and FloatProperty.
   *
   * The XML structure is:
   * \code
   * <PropertyLists>
   *   <PropertyList name="listId">
   *     <Property name="propName" value="..." type="IntProperty"/>
   *   </PropertyList>
   * </PropertyLists>
   * \endcode
   *
   * \sa PersistenceService
   * \sa PropertyList
   */
  class PropertyListsXmlFileReaderAndWriter : public itk::Object
  {
  public:
    /**
     * \brief Get the XML attribute name used for PropertyList identifiers.
     *
     * \return The string "name".
     */
    static const char *GetPropertyListIdElementName();

    mitkClassMacroItkParent(PropertyListsXmlFileReaderAndWriter, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * \brief Write a map of PropertyLists to an XML file.
     *
     * Each PropertyList is serialized with its string identifier. Only BoolProperty,
     * StringProperty, IntProperty, DoubleProperty, and FloatProperty types are supported.
     * Unsupported property types are skipped with a warning.
     *
     * \param[in] fileName Path to the output XML file.
     * \param[in] _PropertyLists Map of identifier-to-PropertyList pairs to serialize.
     * \return True if all properties were converted and the file was written successfully, false otherwise.
     */
    bool WriteLists(const std::string &fileName,
                    const std::map<std::string, mitk::PropertyList::Pointer> &_PropertyLists) const;

    /**
     * \brief Read a map of PropertyLists from an XML file.
     *
     * Parses the XML file and populates the provided map with PropertyList objects
     * keyed by their identifiers. Existing entries in the map may be overwritten.
     *
     * \param[in] fileName Path to the input XML file.
     * \param[in,out] _PropertyLists Map to populate with the deserialized PropertyLists.
     * \return True if at least one PropertyList was read successfully, false on error.
     */
    bool ReadLists(const std::string &fileName,
                   std::map<std::string, mitk::PropertyList::Pointer> &_PropertyLists) const;

  protected:
    PropertyListsXmlFileReaderAndWriter();
    ~PropertyListsXmlFileReaderAndWriter() override;

    bool PropertyFromXmlElem(std::string &name, mitk::BaseProperty::Pointer &prop, const tinyxml2::XMLElement *elem) const;
    bool PropertyToXmlElem(const std::string &name, const mitk::BaseProperty *prop, tinyxml2::XMLElement *elem) const;
  };
}

#endif
