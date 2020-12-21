/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkScalarListLookupTableProperty.h"
#include "mitkScalarListLookupTablePropertySerializer.h"
#include <tinyxml2.h>

tinyxml2::XMLElement* mitk::ScalarListLookupTablePropertySerializer::Serialize(tinyxml2::XMLDocument& doc)
{
  const ScalarListLookupTableProperty* prop =
    dynamic_cast<const ScalarListLookupTableProperty*>(m_Property.GetPointer());

  if (prop == nullptr)
  {
    MITK_ERROR << "Serialization: Property is NULL";
    return nullptr;
  }

  ScalarListLookupTable lut = prop->GetValue();
  const ScalarListLookupTable::LookupTableType& map = lut.GetLookupTable();

  auto* mapElement = doc.NewElement("ScalarListLookupTable");

  for (ScalarListLookupTable::LookupTableType::const_iterator mapIter = map.begin();
     mapIter != map.end(); ++mapIter)
  {
    const ScalarListLookupTable::ValueType& list = mapIter->second;
    auto* listElement = doc.NewElement("List");
    listElement->SetAttribute("name", mapIter->first.c_str());

    for (ScalarListLookupTable::ValueType::const_iterator listIter =  list.begin();
       listIter != list.end(); ++listIter)
    {
      auto* valueElement = doc.NewElement("Element");
      valueElement->SetAttribute("value", *listIter);
      listElement->InsertEndChild(valueElement);
    }

    mapElement->InsertEndChild(listElement);
  }

  return mapElement;
}

mitk::BaseProperty::Pointer
mitk::ScalarListLookupTablePropertySerializer::Deserialize(const tinyxml2::XMLElement* element)
{
  if (!element)
  {
    MITK_ERROR << "Deserialization: Element is NULL";
    return nullptr;
  }

  ScalarListLookupTable lut;

  for (auto* listElement = element->FirstChildElement("List");
     listElement != nullptr; listElement = listElement->NextSiblingElement("List"))
  {
    std::string name;

    if (listElement->Attribute("name") != nullptr)
    {
      name = listElement->Attribute("name");
    }
    else
    {
      MITK_ERROR << "Deserialization: No element with attribute 'name' found";
      return nullptr;
    }

    ScalarListLookupTable::ValueType list;

    for (auto* valueElement = listElement->FirstChildElement("Element");
       valueElement != nullptr;
       valueElement = valueElement->NextSiblingElement("Element"))
    {
      double value;

      if (valueElement->QueryDoubleAttribute("value", &value) != tinyxml2::XML_SUCCESS)
      {
        MITK_ERROR << "Deserialization: No element with attribute 'value' found";
        return nullptr;
      }

      list.push_back(value);
    }

    lut.SetTableValue(name, list);
  }

  return ScalarListLookupTableProperty::New(lut).GetPointer();
}

MITK_REGISTER_SERIALIZER(ScalarListLookupTablePropertySerializer);

::std::string mitk::PropertyPersistenceSerialization::serializeScalarListLookupTablePropertyToXML(
  const mitk::BaseProperty *prop)
{
  mitk::ScalarListLookupTablePropertySerializer::Pointer lutSerializer = mitk::ScalarListLookupTablePropertySerializer::New();

  lutSerializer->SetProperty(prop);

  tinyxml2::XMLDocument doc;
  lutSerializer->Serialize(doc);

  tinyxml2::XMLPrinter printer;
  doc.Print(&printer);

  return printer.CStr();
}

mitk::BaseProperty::Pointer mitk::PropertyPersistenceDeserialization::deserializeXMLToScalarListLookupTableProperty(
  const std::string &value)
{
  mitk::ScalarListLookupTablePropertySerializer::Pointer lutSerializer = mitk::ScalarListLookupTablePropertySerializer::New();

  tinyxml2::XMLDocument doc;
  doc.Parse(value.c_str());
  return lutSerializer->Deserialize(doc.RootElement());
}
