/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPropertyListSerializer_h
#define mitkPropertyListSerializer_h

#include <MitkSceneSerializationBaseExports.h>

#include <mitkPropertyList.h>

#include <itkObjectFactoryBase.h>

namespace tinyxml2
{
  class XMLDocument;
  class XMLElement;
}

namespace mitk
{
  /**
   * \brief Serializes a mitk::PropertyList to an XML file.
   *
   * Iterates over all properties in the assigned PropertyList, uses the
   * appropriate BasePropertySerializer for each property type (discovered
   * via the ITK object factory), and writes the results as an XML file
   * to the working directory.
   *
   * Properties that cannot be serialized (no serializer found, serializer
   * failure, etc.) are collected and can be retrieved via GetFailedProperties().
   *
   * \sa BasePropertySerializer, SceneIO
   */
  class MITKSCENESERIALIZATIONBASE_EXPORT PropertyListSerializer : public itk::Object
  {
  public:
    mitkClassMacroItkParent(PropertyListSerializer, itk::Object);
    itkFactorylessNewMacro(Self) // is this needed? should never be instantiated, only subclasses should
      itkCloneMacro(Self);

      /** \brief Sets a hint for the output filename (without path or extension). */
      itkSetStringMacro(FilenameHint);
    /** \brief Returns the current filename hint. */
    itkGetStringMacro(FilenameHint);

    /** \brief Sets the working directory where the serialized XML file will be stored. */
    itkSetStringMacro(WorkingDirectory);
    /** \brief Returns the current working directory. */
    itkGetStringMacro(WorkingDirectory);

    /**
     * \brief Sets the PropertyList to be serialized.
     * \param[in] _arg Pointer to the PropertyList.
     */
    itkSetObjectMacro(PropertyList, PropertyList);

    /**
     * \brief Serializes the previously set PropertyList to an XML file.
     *
     * Writes an XML file containing all properties from the PropertyList into
     * the working directory. Each property is serialized using its corresponding
     * BasePropertySerializer, discovered via the ITK object factory.
     *
     * \return The filename (without path) of the created XML file, or an empty
     *         string if the PropertyList is null/empty or writing fails.
     *
     * \pre The PropertyList must be set via SetPropertyList().
     * \pre The WorkingDirectory must be set via SetWorkingDirectory().
     *
     * \post Properties that could not be serialized are available via GetFailedProperties().
     */
    virtual std::string Serialize();

    /**
     * \brief Returns properties that failed to serialize during the last Serialize() call.
     *
     * \return Pointer to a PropertyList containing all properties for which serialization
     *         failed, or nullptr if all properties were serialized successfully.
     */
    PropertyList *GetFailedProperties();

  protected:
    PropertyListSerializer();
    ~PropertyListSerializer() override;

    tinyxml2::XMLElement *SerializeOneProperty(tinyxml2::XMLDocument &doc, const std::string &key, const BaseProperty *property);

    std::string m_FilenameHint;
    std::string m_WorkingDirectory;
    PropertyList::Pointer m_PropertyList;

    PropertyList::Pointer m_FailedProperties;
  };

} // namespace

#endif
