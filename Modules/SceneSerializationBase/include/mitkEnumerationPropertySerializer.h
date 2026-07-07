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

#endif
