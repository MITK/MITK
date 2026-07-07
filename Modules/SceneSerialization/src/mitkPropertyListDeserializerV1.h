/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPropertyListDeserializerV1_h
#define mitkPropertyListDeserializerV1_h

#include "mitkPropertyListDeserializer.h"

namespace mitk
{
  /**
   * \brief Deserializes a version 1 mitk::PropertyList from an XML file.
   *
   * Implements the deserialization logic for property list XML files with
   * FileVersion 1. Iterates over all \c \<property\> elements in the XML,
   * looks up the appropriate BasePropertySerializer for each property type
   * via the ITK object factory, and reassembles the PropertyList. The result
   * can be retrieved via GetOutput().
   *
   * This class is registered with the ITK object factory using
   * MITK_REGISTER_SERIALIZER so it can be discovered automatically
   * by PropertyListDeserializer.
   *
   * \sa PropertyListDeserializer, BasePropertySerializer
   */
  class PropertyListDeserializerV1 : public PropertyListDeserializer
  {
  public:
    mitkClassMacro(PropertyListDeserializerV1, PropertyListDeserializer);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self);

    /**
     * \brief Read a version 1 PropertyList from the configured XML file.
     *
     * Parses each \c \<property\> element, instantiates the corresponding serializer
     * based on the type attribute, and deserializes the property value. The resulting
     * PropertyList can be retrieved via GetOutput().
     *
     * \return True if all properties were deserialized successfully, false if any errors occurred.
     *
     * \pre The Filename must be set to a valid version 1 property list XML file.
     */
    bool Deserialize() override;

  protected:
    PropertyListDeserializerV1();
    ~PropertyListDeserializerV1() override;
  };

} // namespace

#endif
