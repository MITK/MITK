/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMultiLabelIOHelper.h"

#include "mitkLabelSetImage.h"
#include <mitkBasePropertySerializer.h>
#include <mitkProperties.h>
#include <mitkStringProperty.h>
#include <mitkCoreServices.h>
#include <mitkIPropertyDeserialization.h>

#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"

#include <tinyxml2.h>

namespace
{
  std::string EnsureExtension(const std::string& filename)
  {
    const std::string extension = ".lsetp";

    if (filename.size() < extension.size() || std::string::npos == filename.find(extension, filename.size() - extension.size()))
      return filename + extension;

    return filename;
  }
}

bool mitk::MultiLabelIOHelper::SaveLabelSetImagePreset(const std::string &presetFilename,
                                                     const mitk::MultiLabelSegmentation *inputImage)
{
  const auto filename = EnsureExtension(presetFilename);

  tinyxml2::XMLDocument xmlDocument;
  xmlDocument.InsertEndChild(xmlDocument.NewDeclaration());

  auto *rootElement = xmlDocument.NewElement("LabelSetImagePreset");
  rootElement->SetAttribute("layers", inputImage->GetNumberOfGroups());
  xmlDocument.InsertEndChild(rootElement);

  for (unsigned int layerIndex = 0; layerIndex < inputImage->GetNumberOfGroups(); layerIndex++)
  {
    auto *layerElement = xmlDocument.NewElement("Layer");
    layerElement->SetAttribute("index", layerIndex);
    layerElement->SetAttribute("labels", inputImage->GetNumberOfLabels(layerIndex));
    rootElement->InsertEndChild(layerElement);

    auto labelsInGroup = inputImage->GetConstLabelsByValue(inputImage->GetLabelValuesByGroup(layerIndex));

    for (const auto& label : labelsInGroup)
      layerElement->InsertEndChild(MultiLabelIOHelper::GetLabelAsXMLElement(xmlDocument, label));
  }

  return tinyxml2::XML_SUCCESS == xmlDocument.SaveFile(filename.c_str());
}

bool mitk::MultiLabelIOHelper::LoadLabelSetImagePreset(const std::string &presetFilename,
                                                     mitk::MultiLabelSegmentation *inputImage)
{
  if (nullptr == inputImage)
    return false;

  const auto filename = EnsureExtension(presetFilename);

  tinyxml2::XMLDocument xmlDocument;

  if (tinyxml2::XML_SUCCESS != xmlDocument.LoadFile(filename.c_str()))
  {
    MITK_WARN << "Label set preset file \"" << filename << "\" does not exist or cannot be opened";
    return false;
  }

  auto *rootElement = xmlDocument.FirstChildElement("LabelSetImagePreset");

  if (nullptr == rootElement)
  {
    MITK_WARN << "Not a valid Label set preset";
    return false;
  }

  auto activeLabelBackup = inputImage->GetActiveLabel();

  int numberOfLayers = 0;
  rootElement->QueryIntAttribute("layers", &numberOfLayers);

  auto* layerElement = rootElement->FirstChildElement("Layer");

  if (nullptr == layerElement)
  {
    MITK_WARN << "Label set preset does not contain any layers";
    return false;
  }

  for (int layerIndex = 0; layerIndex < numberOfLayers; layerIndex++)
  {
    int numberOfLabels = 0;
    layerElement->QueryIntAttribute("labels", &numberOfLabels);

    if (!inputImage->ExistGroup(layerIndex))
    {
      while (!inputImage->ExistGroup(layerIndex))
      {
        inputImage->AddGroup();
      }
    }

    auto *labelElement = layerElement->FirstChildElement("Label");

    if (nullptr == labelElement)
      continue;

    for (int labelIndex = 0; labelIndex < numberOfLabels; labelIndex++)
    {
      auto label = mitk::MultiLabelIOHelper::LoadLabelFromXMLDocument(labelElement);
      const auto labelValue = label->GetValue();

      if (MultiLabelSegmentation::UNLABELED_VALUE != labelValue)
      {
        if (inputImage->ExistLabel(labelValue))
        {
          // Override existing label with label from preset
          auto alreadyExistingLabel = inputImage->GetLabel(labelValue);
          alreadyExistingLabel->ConcatenatePropertyList(label);
          inputImage->UpdateLookupTable(labelValue);
        }
        else
        {
          inputImage->AddLabel(label, layerIndex, false);
        }
      }

      labelElement = labelElement->NextSiblingElement("Label");

      if (nullptr == labelElement)
        continue;
    }

    layerElement = layerElement->NextSiblingElement("Layer");

    if (nullptr == layerElement)
      continue;
  }

  if (nullptr != activeLabelBackup)
  {
    inputImage->SetActiveLabel(activeLabelBackup->GetValue());
  }

  return true;
}

tinyxml2::XMLElement *mitk::MultiLabelIOHelper::GetLabelAsXMLElement(tinyxml2::XMLDocument &doc, const Label *label)
{
  auto *labelElem = doc.NewElement("Label");

  if (nullptr != label)
  {
    // add XML contents
    const PropertyList::PropertyMap* propmap = label->GetMap();
    for (auto iter = propmap->begin(); iter != propmap->end(); ++iter)
    {
      std::string key = iter->first;
      const BaseProperty* property = iter->second;
      auto* element = PropertyToXMLElement(doc, key, property);
      if (element)
        labelElem->InsertEndChild(element);
    }
  }

  return labelElem;
}

mitk::Label::Pointer mitk::MultiLabelIOHelper::LoadLabelFromXMLDocument(const tinyxml2::XMLElement *labelElem)
{
  // reread
  auto *propElem = labelElem->FirstChildElement("property");

  std::string name;
  mitk::BaseProperty::Pointer prop;

  mitk::Label::Pointer label = mitk::Label::New();
  while (propElem)
  {
    MultiLabelIOHelper::PropertyFromXMLElement(name, prop, propElem);
    label->SetProperty(name, prop);
    propElem = propElem->NextSiblingElement("property");
  }

  return label.GetPointer();
}

tinyxml2::XMLElement *mitk::MultiLabelIOHelper::PropertyToXMLElement(tinyxml2::XMLDocument &doc, const std::string &key, const BaseProperty *property)
{
  auto *keyelement = doc.NewElement("property");
  keyelement->SetAttribute("key", key.c_str());
  keyelement->SetAttribute("type", property->GetNameOfClass());

  // construct name of serializer class
  std::string serializername(property->GetNameOfClass());
  serializername += "Serializer";

  std::list<itk::LightObject::Pointer> allSerializers =
    itk::ObjectFactoryBase::CreateAllInstance(serializername.c_str());
  if (allSerializers.size() < 1)
    MITK_ERROR << "No serializer found for " << property->GetNameOfClass() << ". Skipping object";

  if (allSerializers.size() > 1)
    MITK_WARN << "Multiple serializers found for " << property->GetNameOfClass() << "Using arbitrarily the first one.";

  for (auto iter = allSerializers.begin(); iter != allSerializers.end();
       ++iter)
  {
    if (auto *serializer = dynamic_cast<BasePropertySerializer *>(iter->GetPointer()))
    {
      serializer->SetProperty(property);
      try
      {
        auto *valueelement = serializer->Serialize(doc);
        if (valueelement)
          keyelement->InsertEndChild(valueelement);
      }
      catch (std::exception &e)
      {
        MITK_ERROR << "Serializer " << serializer->GetNameOfClass() << " failed: " << e.what();
      }
      break;
    }
  }
  return keyelement;
}

bool mitk::MultiLabelIOHelper::PropertyFromXMLElement(std::string &key,
                                                    mitk::BaseProperty::Pointer &prop,
                                                    const tinyxml2::XMLElement *elem)
{
  const char* typeC = elem->Attribute("type");
  std::string type = nullptr != typeC
    ? typeC
    : "";

  const char* keyC = elem->Attribute("key");
  key = nullptr != keyC
    ? keyC
    : "";

  // construct name of serializer class
  std::string serializername(type);
  serializername += "Serializer";

  std::list<itk::LightObject::Pointer> allSerializers =
    itk::ObjectFactoryBase::CreateAllInstance(serializername.c_str());
  if (allSerializers.size() < 1)
    MITK_ERROR << "No serializer found for " << type << ". Skipping object";

  if (allSerializers.size() > 1)
    MITK_WARN << "Multiple deserializers found for " << type << "Using arbitrarily the first one.";

  for (auto iter = allSerializers.begin(); iter != allSerializers.end();
       ++iter)
  {
    if (auto *serializer = dynamic_cast<BasePropertySerializer *>(iter->GetPointer()))
    {
      try
      {
        prop = serializer->Deserialize(elem->FirstChildElement());
      }
      catch (std::exception &e)
      {
        MITK_ERROR << "Deserializer " << serializer->GetNameOfClass() << " failed: " << e.what();
        return false;
      }
      break;
    }
  }
  if (prop.IsNull())
    return false;
  return true;
}

int mitk::MultiLabelIOHelper::GetIntByKey(const itk::MetaDataDictionary& dic, const std::string& str)
{
  std::vector<std::string> imgMetaKeys = dic.GetKeys();
  std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
  std::string metaString("");
  for (; itKey != imgMetaKeys.end(); itKey++)
  {
    itk::ExposeMetaData<std::string>(dic, *itKey, metaString);
    if (itKey->find(str.c_str()) != std::string::npos)
    {
      return atoi(metaString.c_str());
    }
  }
  return 0;
}

std::string mitk::MultiLabelIOHelper::GetStringByKey(const itk::MetaDataDictionary& dic, const std::string& str)
{
  std::vector<std::string> imgMetaKeys = dic.GetKeys();
  std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
  std::string metaString("");
  for (; itKey != imgMetaKeys.end(); itKey++)
  {
    itk::ExposeMetaData<std::string>(dic, *itKey, metaString);
    if (itKey->find(str.c_str()) != std::string::npos)
    {
      return metaString;
    }
  }
  return metaString;
}

mitk::MultiLabelIOHelper::LabelGroupMetaData::LabelGroupMetaData() : properties(PropertyList::New())
{};

mitk::MultiLabelIOHelper::LabelGroupMetaData::LabelGroupMetaData(const std::string& groupName, const mitk::LabelVector& groupLabels)
  : name(groupName), labels(groupLabels), properties(PropertyList::New())
{};

mitk::MultiLabelIOHelper::LabelGroupMetaData::LabelGroupMetaData(const std::string& groupName, const mitk::LabelVector& groupLabels, PropertyList* groupProperties)
  : name(groupName), labels(groupLabels), properties(groupProperties)
{};

nlohmann::json mitk::MultiLabelIOHelper::SerializeMultLabelGroupsToJSON(const mitk::MultiLabelSegmentation* inputImage)
{
  if (nullptr == inputImage)
  {
    mitkThrow() << "Invalid call of SerializeMultLabelGroupsToJSON. Passed image pointer is null.";
  }

  nlohmann::json result;

  for (MultiLabelSegmentation::GroupIndexType i = 0; i < inputImage->GetNumberOfGroups(); i++)
  {
    nlohmann::json jgroup;
    nlohmann::json jlabels;

    for (const auto& label : inputImage->GetConstLabelsByValue(inputImage->GetLabelValuesByGroup(i)))
    {
      jlabels.emplace_back(SerializeLabelToJSON(label));
    }

    jgroup["labels"] = jlabels;

    const auto& name = inputImage->GetGroupName(i);
    if (!name.empty())
    {
      jgroup["name"] = name;
    }

    result.emplace_back(jgroup);
  }
  return result;
};

template<typename TValueType> bool GetValueFromJson(const nlohmann::json& labelJson, const std::string& key, TValueType& value)
{
  if (labelJson.find(key) != labelJson.end())
  {
    try
    {
      value = labelJson[key].get<TValueType>();
      return true;
    }
    catch (...)
    {
      MITK_ERROR << "Unable to read label information from json. Value has wrong type. Failed key: " << key << "; invalid value: " << labelJson[key].dump();
      throw;
    }
  }
  return false;
}

std::vector<mitk::MultiLabelIOHelper::LabelGroupMetaData> mitk::MultiLabelIOHelper::DeserializeMultiLabelGroupsFromJSON(const nlohmann::json& listOfLabelGroups)
{
  std::vector<LabelGroupMetaData> result;

  for (const auto& jLabelGroup : listOfLabelGroups)
  {
    LabelVector labels;
    std::string name;
    PropertyList::Pointer customProps;

    for (const auto& [key, jValue] : jLabelGroup.items())
    {
      if (key=="labels")
      {
        auto jlabels = jLabelGroup["labels"];

        for (const auto& jlabel : jlabels)
        {
          labels.push_back(DeserializeLabelFromJSON(jlabel));
        }
      }
      else if (key=="name")
      {
        GetValueFromJson(jLabelGroup, "name", name);
      }
      else
      {
        if (jValue.is_string())
        {
          customProps->SetStringProperty(key.c_str(), jValue.get<std::string>().c_str());
        }
        else if (jValue.is_number_integer())
        {
          customProps->SetIntProperty(key.c_str(), jValue.get<int>());
        }
        else if (jValue.is_number_float())
        {
          customProps->SetFloatProperty(key.c_str(), jValue.get<float>());
        }
        else if (jValue.is_boolean())
        {
          customProps->SetBoolProperty(key.c_str(), jValue.get<bool>());
        }
        else
        {
          MITK_ERROR << "Unable to read custom group property from JSON. Value has wrong type. Failed key: " << key << "; invalid value: " << jValue.dump();
          mitkThrow() << "Unable to read custom group property from JSON. Value has wrong type. Failed key: " << key << "; invalid value: " << jValue.dump();
        }
      }
    }

    result.emplace_back(name,labels, customProps);
  }

  return result;
}


void SerializeLabelCustomPropertiesToJSON(const mitk::Label* label, nlohmann::json& j)
{
  // Get all properties from the label
  const auto* propMap = label->GetMap();

  // Define reserved property keys that are handled explicitly
  static const std::set<std::string> reservedKeys = {
    "name", "locked", "opacity", "visible", "color", "value",
    "tracking_id", "tracking_uid", "description"
  };

  for (const auto& [key, property] : *propMap)
  {
    // Skip reserved keys
    if (reservedKeys.find(key) != reservedKeys.end())
      continue;

    // Try to serialize the property to JSON
    j[key] = mitk::MultiLabelIOHelper::SerializeLabelPropertyToJSON(property);
  }
}

nlohmann::json mitk::MultiLabelIOHelper::SerializeLabelPropertyToJSON(const BaseProperty* property)
{
  if (property == nullptr)
  {
    mitkThrow() << "Invalid call of SerializeLabelPropertyToJSON. Passed property pointer is null.";
  }

  // Try to handle common property types directly
  if (auto stringProp = dynamic_cast<const StringProperty*>(property); nullptr!=stringProp)
  {
    return stringProp->GetValueAsString();
  }
  else if (auto intProp = dynamic_cast<const IntProperty*>(property); nullptr != intProp)
  {
    return intProp->GetValue();
  }
  else if (auto floatProp = dynamic_cast<const FloatProperty*>(property); nullptr != floatProp)
  {
    return floatProp->GetValue();
  }
  else if (auto boolProp = dynamic_cast<const BoolProperty*>(property); nullptr != boolProp)
  {
    return boolProp->GetValue();
  }

  // For complex properties, store with type information
  nlohmann::json propJson;
  propJson["type"] = property->GetNameOfClass();

  nlohmann::json valueJson;
  property->ToJSON(valueJson);
  propJson["value"] = valueJson;
  return propJson;
}

nlohmann::json mitk::MultiLabelIOHelper::SerializeLabelToJSON(const Label* label)
{
  if (nullptr == label)
  {
    mitkThrow() << "Invalid call of GetLabelAsJSON. Passed label pointer is null.";
  }

  nlohmann::json j;
  j["name"] = label->GetName();

  j["value"] = label->GetValue();

  j["color"] = { label->GetColor().GetRed(), label->GetColor().GetGreen(), label->GetColor().GetBlue() };

  j["locked"] = label->GetLocked();
  j["opacity"] = label->GetOpacity();
  j["visible"] = label->GetVisible();
  j["tracking_id"] = label->GetTrackingID();

  if (!label->GetTrackingUID().empty())
    j["tracking_uid"] = label->GetTrackingUID();
  if (!label->GetDescription().empty())
    j["description"] = label->GetDescription();

  // Serialize any other custom properties
  SerializeLabelCustomPropertiesToJSON(label, j);

  return j;
};

mitk::BaseProperty::Pointer CreatePropertyFromJSON(const std::string& typeStr, const nlohmann::json& serializedPropertyValue)
{
  mitk::CoreServicePointer<mitk::IPropertyDeserialization> service(mitk::CoreServices::GetPropertyDeserialization());
  auto property = service->CreateInstance(typeStr);

  if (property.IsNull())
  {
    MITK_ERROR << "Cannot create property instance of class \"" << typeStr << "\"!";
    return nullptr;
  }

  property->FromJSON(serializedPropertyValue);

  return property;
}

mitk::Label::Pointer mitk::MultiLabelIOHelper::DeserializeLabelFromJSON(const nlohmann::json& labelJson)
{
  Label::Pointer resultLabel = Label::New();

  for (const auto& [key, jValue] : labelJson.items())
  {
    if (key == "name")
    {
      std::string name;
      if (GetValueFromJson(labelJson, "name", name))
        resultLabel->SetName(name);
    }
    else if (key == "value")
    {
      Label::PixelType value = 1;
      if (GetValueFromJson(labelJson, "value", value))
        resultLabel->SetValue(value);
    }
    else if (key == "color")
    {
      Color color;
      if (jValue.contains("value"))
      { // "old" property passed serialization
        auto jcolor = jValue["value"];
        color.SetRed(jcolor[0].get<float>());
        color.SetGreen(jcolor[1].get<float>());
        color.SetBlue(jcolor[2].get<float>());
      }
      else
      { // simple new serialization directly as array
        color.SetRed(jValue[0].get<float>());
        color.SetGreen(jValue[1].get<float>());
        color.SetBlue(jValue[2].get<float>());
      }
      resultLabel->SetColor(color);
    }
    else if (key == "locked")
    {
      bool locked = false;
      if (GetValueFromJson(labelJson, "locked", locked))
        resultLabel->SetLocked(locked);
    }
    else if (key == "opacity")
    {
      float opacity = 1.;
      if (GetValueFromJson(labelJson, "opacity", opacity))
        resultLabel->SetOpacity(opacity);
    }
    else if (key == "visible")
    {
      bool visible = true;
      if (GetValueFromJson(labelJson, "visible", visible))
        resultLabel->SetVisible(visible);
    }
    else if (key == "tracking_uid")
    {
      std::string tracking_uid;
      if (GetValueFromJson(labelJson, "tracking_uid", tracking_uid))
        resultLabel->SetTrackingUID(tracking_uid);
    }
    else if (key == "tracking_id")
    {
      std::string tracking_id;
      if (GetValueFromJson(labelJson, "tracking_id", tracking_id))
        resultLabel->SetTrackingID(tracking_id);
    }
    else if (key == "description")
    {
      std::string description;
      if (GetValueFromJson(labelJson, "description", description))
        resultLabel->SetDescription(description);
    }
    else
    { //unknown custom key that we just store as additional property
      if (jValue.contains("type"))
      { // full property specification
        auto property = CreatePropertyFromJSON(jValue["type"], jValue["value"]);
        resultLabel->SetProperty(key, property);
      }
      else
      { // support for direct simple types
        if (jValue.is_string())
        {
          resultLabel->SetStringProperty(key.c_str(), jValue.get<std::string>().c_str());
        }
        else if (jValue.is_number_integer())
        {
          resultLabel->SetIntProperty(key.c_str(), jValue.get<int>());
        }
        else if (jValue.is_number_float())
        {
          resultLabel->SetFloatProperty(key.c_str(), jValue.get<float>());
        }
        else if (jValue.is_boolean())
        {
          resultLabel->SetBoolProperty(key.c_str(), jValue.get<bool>());
        }
        else
        {
          MITK_ERROR << "Unable to read custom label property from JSON. Value has wrong type. Failed key: " << key << "; invalid value: " << jValue.dump();
          mitkThrow() << "Unable to read custom label property from JSON. Value has wrong type. Failed key: " << key << "; invalid value: " << jValue.dump();
        }
      }
    }
  }

  return resultLabel;
}
