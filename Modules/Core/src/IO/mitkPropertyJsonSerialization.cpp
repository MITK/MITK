/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPropertyJsonSerialization.h"

#include <mitkCoreServices.h>
#include <mitkException.h>
#include <mitkIPropertyDeserialization.h>
#include <mitkLog.h>
#include <mitkProperties.h>
#include <mitkStringProperty.h>

namespace mitk
{
namespace PropertyJsonSerialization
{

nlohmann::json ToJson(const BaseProperty* property)
{
  if (property == nullptr)
  {
    mitkThrow() << "Invalid call of PropertyJsonSerialization::ToJson. Passed property pointer is null.";
  }

  // Try to handle common simple property types directly
  // Note: DoubleProperty is intentionally NOT handled here to avoid conversion
  // inconsistency (JSON float would deserialize as FloatProperty, not DoubleProperty)
  if (auto stringProp = dynamic_cast<const StringProperty*>(property); stringProp != nullptr)
  {
    return stringProp->GetValueAsString();
  }
  else if (auto intProp = dynamic_cast<const IntProperty*>(property); intProp != nullptr)
  {
    return intProp->GetValue();
  }
  else if (auto floatProp = dynamic_cast<const FloatProperty*>(property); floatProp != nullptr)
  {
    return floatProp->GetValue();
  }
  else if (auto boolProp = dynamic_cast<const BoolProperty*>(property); boolProp != nullptr)
  {
    return boolProp->GetValue();
  }

  // For complex properties, store with type information
  nlohmann::json propJson;
  propJson["type"] = property->GetNameOfClass();

  nlohmann::json valueJson;
  if (!property->ToJSON(valueJson))
  {
    mitkThrow() << "Property type \"" << property->GetNameOfClass()
                << "\" does not support JSON serialization.";
  }

  propJson["value"] = valueJson;
  return propJson;
}

BaseProperty::Pointer FromJson(const nlohmann::json& json)
{
  // Check if this is a complex property with type information
  if (json.is_object() && json.contains("type"))
  {
    std::string typeStr = json["type"].get<std::string>();

    CoreServicePointer<IPropertyDeserialization> service(CoreServices::GetPropertyDeserialization());
    if (!service)
    {
      mitkThrow() << "IPropertyDeserialization service not available.";
    }

    auto property = service->CreateInstance(typeStr);
    if (property.IsNull())
    {
      mitkThrow() << "Cannot create property instance of class \"" << typeStr << "\".";
    }

    if (json.contains("value"))
    {
      if (!property->FromJSON(json["value"]))
      {
        mitkThrow() << "Property type \"" << typeStr << "\" failed to deserialize from JSON value.";
      }
    }

    return property;
  }

  // Handle simple JSON types
  if (json.is_string())
  {
    return StringProperty::New(json.get<std::string>());
  }
  else if (json.is_number_integer())
  {
    return IntProperty::New(json.get<int>());
  }
  else if (json.is_number_float())
  {
    return FloatProperty::New(json.get<float>());
  }
  else if (json.is_boolean())
  {
    return BoolProperty::New(json.get<bool>());
  }

  // Unsupported JSON type
  mitkThrow() << "Unable to deserialize property from JSON. Unsupported JSON type: " << json.type_name();
}

nlohmann::json PropertyListToJson(const PropertyList* propertyList)
{
  if (propertyList == nullptr)
  {
    mitkThrow() << "Invalid call of PropertyJsonSerialization::PropertyListToJson. "
                << "Passed propertyList pointer is null.";
  }

  nlohmann::json result = nlohmann::json::object();

  const PropertyList::PropertyMap* propMap = propertyList->GetMap();
  for (const auto& [key, property] : *propMap)
  {
    result[key] = ToJson(property);
  }

  return result;
}

PropertyList::Pointer PropertyListFromJson(const nlohmann::json& json)
{
  if (!json.is_object())
  {
    mitkThrow() << "Invalid JSON for PropertyListFromJson. Expected JSON object.";
  }

  auto propertyList = PropertyList::New();

  for (const auto& [key, value] : json.items())
  {
    auto property = FromJson(value);
    propertyList->SetProperty(key, property);
  }

  return propertyList;
}

} // namespace PropertyJsonSerialization
} // namespace mitk
