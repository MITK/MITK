/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkScalarListLookupTableProperty.h"
#include "mitkScalarListLookupTablePropertySerializer.h"


TiXmlElement* mitk::ScalarListLookupTablePropertySerializer::Serialize()
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

	TiXmlElement* mapElement = new TiXmlElement("ScalarListLookupTable");

	for (ScalarListLookupTable::LookupTableType::const_iterator mapIter = map.begin();
		 mapIter != map.end(); ++mapIter)
	{
		const ScalarListLookupTable::ValueType& list = mapIter->second;
		TiXmlElement* listElement = new TiXmlElement("List");
		listElement->SetAttribute("name", mapIter->first);

		for (ScalarListLookupTable::ValueType::const_iterator listIter =  list.begin();
			 listIter != list.end(); ++listIter)
		{
			TiXmlElement* valueElement = new TiXmlElement("Element");
			valueElement->SetDoubleAttribute("value", *listIter);
			listElement->LinkEndChild(valueElement);
		}

		mapElement->LinkEndChild(listElement);
	}

	return mapElement;
}

mitk::BaseProperty::Pointer
mitk::ScalarListLookupTablePropertySerializer::Deserialize(TiXmlElement* element)
{
	if (!element)
	{
		MITK_ERROR << "Deserialization: Element is NULL";
		return nullptr;
	}

	ScalarListLookupTable lut;

	for (TiXmlElement* listElement = element->FirstChildElement("List");
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

		for (TiXmlElement* valueElement = listElement->FirstChildElement("Element");
			 valueElement != nullptr;
			 valueElement = valueElement->NextSiblingElement("Element"))
		{
			double value;

			if (valueElement->QueryDoubleAttribute("value", &value) == TIXML_WRONG_TYPE)
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
  auto xmlLut = lutSerializer->Serialize();

  TiXmlPrinter printer;
  xmlLut->Accept(&printer);
  printer.SetStreamPrinting();
  return printer.Str();
}

mitk::BaseProperty::Pointer mitk::PropertyPersistenceDeserialization::deserializeXMLToScalarListLookupTableProperty(
  const std::string &value)
{
  mitk::ScalarListLookupTablePropertySerializer::Pointer lutSerializer = mitk::ScalarListLookupTablePropertySerializer::New();

  TiXmlDocument doc;
  doc.Parse(value.c_str());
  return lutSerializer->Deserialize(doc.RootElement());
}
