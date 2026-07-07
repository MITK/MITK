/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPropertyListDeserializer_h
#define mitkPropertyListDeserializer_h

#include <mitkPropertyList.h>

namespace mitk
{
  /**
   * \brief Deserializes a mitk::PropertyList from an XML file.
   *
   * Acts as a version-dispatching base class for property list deserialization.
   * When Deserialize() is called, it reads the file version from the XML document
   * and instantiates the appropriate versioned deserializer (e.g., PropertyListDeserializerV1)
   * through the ITK object factory mechanism. The deserialized PropertyList can then be
   * retrieved via GetOutput().
   *
   * \sa PropertyListDeserializerV1, PropertyListSerializer, SceneIO
   */
  class PropertyListDeserializer : public itk::Object
  {
  public:
    mitkClassMacroItkParent(PropertyListDeserializer, itk::Object);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self);

    /** \brief Set the filename of the XML file to deserialize. */
    itkSetStringMacro(Filename);
    /** \brief Get the filename of the XML file to deserialize. */
    itkGetStringMacro(Filename);

    /**
     * \brief Read a PropertyList from the configured XML file.
     *
     * Parses the file to determine its version, then delegates to the appropriate
     * versioned deserializer found via the ITK object factory. The result can be
     * retrieved with GetOutput().
     *
     * \return True if deserialization succeeded, false otherwise.
     *
     * \pre The Filename must be set to a valid XML file path.
     */
    virtual bool Deserialize();

    /**
     * \brief Retrieve the deserialized PropertyList.
     *
     * \return The PropertyList read from the file, or nullptr if Deserialize()
     *         has not been called or failed.
     */
    virtual PropertyList::Pointer GetOutput();

  protected:
    PropertyListDeserializer();
    ~PropertyListDeserializer() override;

    std::string m_Filename;           ///< \brief Path to the XML file to deserialize.
    PropertyList::Pointer m_PropertyList; ///< \brief The deserialized property list.
  };

} // namespace

#endif
