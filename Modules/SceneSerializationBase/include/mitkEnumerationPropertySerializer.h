/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkEnumerationPropertySerializer_h
#define mitkEnumerationPropertySerializer_h

#include <mitkBasePropertySerializer.h>

#include <mitkEnumerationProperty.h>
#include <mitkLog.h>

namespace mitk
{
  /**
   * \brief Serializer for mitk::EnumerationProperty and its sub-classes.
   *
   * Serializes an EnumerationProperty into an XML element with the tag "enum"
   * and the current enumeration value as a string attribute. This serializer
   * serves as the base for all enumeration-type property serializers.
   *
   * \sa BasePropertySerializer, EnumerationProperty
   */
  class MITKSCENESERIALIZATIONBASE_EXPORT EnumerationPropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(EnumerationPropertySerializer, BasePropertySerializer);

    /**
     * \brief Serializes the EnumerationProperty into an XML element.
     *
     * Creates an XML element with tag "enum" and a "value" attribute set to
     * the string representation of the current enumeration value.
     *
     * \param[in,out] doc The XML document used to create the element.
     * \return Pointer to the created XML element, or nullptr if the property
     *         is not an EnumerationProperty.
     */
    tinyxml2::XMLElement *Serialize(tinyxml2::XMLDocument& doc) override;

  protected:
    EnumerationPropertySerializer();
    ~EnumerationPropertySerializer() override;
  };

} // namespace

/**
 * \brief Defines and registers a serializer for a concrete EnumerationProperty subclass.
 *
 * The serializer lookup is by class name, so every EnumerationProperty subclass
 * needs its own serializer even though the XML format is shared. This macro
 * generates that serializer: serialization is inherited from
 * EnumerationPropertySerializer, deserialization instantiates \a classname and
 * sets the stored value by its name. A name the class does not know fails the
 * deserialization instead of silently yielding the default value.
 *
 * Use it at global scope in a .cpp file that includes the property's header
 * and tinyxml2.h.
 *
 * \param classname The unqualified name of the EnumerationProperty subclass.
 *                  The class must reside in the mitk namespace.
 */
#define MITK_REGISTER_ENUM_SUB_SERIALIZER(classname)                                        \
                                                                                            \
namespace mitk                                                                              \
{                                                                                           \
  class classname##Serializer : public EnumerationPropertySerializer                        \
  {                                                                                         \
  public:                                                                                   \
    mitkClassMacro(classname##Serializer, EnumerationPropertySerializer)                    \
    itkFactorylessNewMacro(Self)                                                            \
    itkCloneMacro(Self)                                                                     \
                                                                                            \
    BaseProperty::Pointer Deserialize(const tinyxml2::XMLElement *element) override         \
    {                                                                                       \
      const char *value = nullptr != element ? element->Attribute("value") : nullptr;      \
                                                                                            \
      if (nullptr == value)                                                                 \
        return nullptr;                                                                     \
                                                                                            \
      classname::Pointer property = classname::New();                                       \
                                                                                            \
      if (!property->SetValue(value))                                                       \
      {                                                                                     \
        MITK_ERROR << "Unknown " #classname " value: " << value;                            \
        return nullptr;                                                                     \
      }                                                                                     \
                                                                                            \
      return property.GetPointer();                                                         \
    }                                                                                       \
                                                                                            \
  protected:                                                                                \
    classname##Serializer() {}                                                              \
    ~classname##Serializer() override {}                                                    \
  };                                                                                        \
}                                                                                           \
                                                                                            \
MITK_REGISTER_SERIALIZER(classname##Serializer);

#endif
