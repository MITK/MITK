/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPropertyJsonSerialization_h
#define mitkPropertyJsonSerialization_h

#include <MitkCoreExports.h>
#include <mitkBaseProperty.h>
#include <mitkPropertyList.h>
#include <nlohmann/json.hpp>

namespace mitk
{
  /**
   * @brief Utility functions for Property <-> JSON conversion.
   *
   * Provides generic serialization for MITK properties to JSON format.
   *
   * JSON format:
   * - Simple types (string, int, float, bool) serialize directly as their JSON value
   * - Complex types use: {"type": "ClassName", "value": {...}}
   *
   * @ingroup DataManagement
   */
  namespace PropertyJsonSerialization
  {
    /**
     * @brief Serialize a property to JSON.
     *
     * Simple property types (StringProperty, IntProperty, FloatProperty, BoolProperty)
     * are serialized directly as their JSON primitive values.
     * Complex types are serialized as objects with "type" and "value" fields.
     *
     * @param property The property to serialize. Must not be nullptr.
     * @return JSON representation of the property.
     * @throws mitk::Exception if property is nullptr or does not support JSON serialization.
     */
    MITKCORE_EXPORT nlohmann::json ToJson(const BaseProperty* property);

    /**
     * @brief Deserialize a property from JSON.
     *
     * Handles both simple JSON values (string, int, float, bool) and complex
     * property objects with "type" and "value" fields.
     *
     * For simple values, the type is inferred from the JSON type:
     * - JSON string -> StringProperty
     * - JSON integer -> IntProperty
     * - JSON float -> FloatProperty
     * - JSON boolean -> BoolProperty
     *
     * For complex types, uses IPropertyDeserialization service to create instances.
     *
     * @param json The JSON value to deserialize.
     * @return Created property.
     * @throws mitk::Exception if JSON type is unsupported or deserialization fails.
     */
    MITKCORE_EXPORT BaseProperty::Pointer FromJson(const nlohmann::json& json);

    /**
     * @brief Serialize all properties from a PropertyList to JSON.
     *
     * Creates a JSON object where each key is a property name and each value
     * is the serialized property (using ToJson).
     *
     * @param propertyList The list to serialize. Must not be nullptr.
     * @return JSON object with property names as keys.
     * @throws mitk::Exception if propertyList is nullptr or a property fails to serialize.
     */
    MITKCORE_EXPORT nlohmann::json PropertyListToJson(const PropertyList* propertyList);

    /**
     * @brief Deserialize properties from JSON into a new PropertyList.
     *
     * Creates a new PropertyList and populates it with properties
     * deserialized from the JSON object.
     *
     * @param json Source JSON object containing properties.
     * @return New PropertyList containing the deserialized properties.
     * @throws mitk::Exception if JSON is not an object or a property fails to deserialize.
     */
    MITKCORE_EXPORT PropertyList::Pointer PropertyListFromJson(const nlohmann::json& json);
  }
}

#endif
