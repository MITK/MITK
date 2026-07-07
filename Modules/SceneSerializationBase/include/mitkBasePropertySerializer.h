/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBasePropertySerializer_h
#define mitkBasePropertySerializer_h

#include <mitkSerializerMacros.h>
#include <MitkSceneSerializationBaseExports.h>

#include <mitkBaseProperty.h>
#include <itkObjectFactoryBase.h>

namespace tinyxml2
{
  class XMLDocument;
  class XMLElement;
}

namespace mitk
{
  /**
   * \brief Base class for objects that serialize and deserialize BaseProperty types to/from XML.
   *
   * Provides the interface for converting mitk::BaseProperty-derived objects
   * to and from XML elements (using TinyXML2). Sub-classes implement the
   * actual serialization and deserialization logic for specific property types.
   *
   * The naming convention for sub-classes must follow this pattern:
   * \verbatim
   * If the class derived from BaseProperty is called GreenProperty
   * Then the serializer for this class must be called GreenPropertySerializer
   * \endverbatim
   *
   * Sub-classes are discovered at runtime through the ITK object factory mechanism.
   * Use the MITK_REGISTER_SERIALIZER macro to register a new serializer.
   *
   * \sa BaseDataSerializer, PropertyListSerializer, MITK_REGISTER_SERIALIZER
   */
  class MITKSCENESERIALIZATIONBASE_EXPORT BasePropertySerializer : public itk::Object
  {
  public:
    mitkClassMacroItkParent(BasePropertySerializer, itk::Object);

    /**
     * \brief Sets the property to be serialized.
     * \param[in] _arg Pointer to the BaseProperty object.
     */
    itkSetConstObjectMacro(Property, BaseProperty);

    /**
     * \brief Serializes the previously set BaseProperty object into an XML element.
     *
     * The base implementation logs the request and returns nullptr.
     * Sub-classes must override this method to produce a meaningful XML representation.
     *
     * \param[in,out] doc The XML document used to create new elements.
     * \return A pointer to the newly created XML element representing the property,
     *         or nullptr if serialization is not implemented or fails.
     *
     * \pre The property must be set via SetProperty() before calling this method.
     */
    virtual tinyxml2::XMLElement *Serialize(tinyxml2::XMLDocument& doc);

    /**
     * \brief Deserializes an XML element back into a BaseProperty object.
     *
     * The base implementation logs an error and returns nullptr.
     * Sub-classes must override this method to reconstruct a property from XML.
     *
     * \param[in] element The XML element to deserialize. May be nullptr.
     * \return A smart pointer to the deserialized property, or nullptr if
     *         deserialization is not implemented or fails.
     */
    virtual BaseProperty::Pointer Deserialize(const tinyxml2::XMLElement* element);

  protected:
    BasePropertySerializer();
    ~BasePropertySerializer() override;

    BaseProperty::ConstPointer m_Property;
  };

} // namespace

#endif
