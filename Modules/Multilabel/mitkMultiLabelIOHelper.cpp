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
                                                     const mitk::LabelSetImage *inputImage)
{
  const auto filename = EnsureExtension(presetFilename);

  tinyxml2::XMLDocument xmlDocument;
  xmlDocument.InsertEndChild(xmlDocument.NewDeclaration());

  auto *rootElement = xmlDocument.NewElement("LabelSetImagePreset");
  rootElement->SetAttribute("layers", inputImage->GetNumberOfLayers());
  xmlDocument.InsertEndChild(rootElement);

  for (unsigned int layerIndex = 0; layerIndex < inputImage->GetNumberOfLayers(); layerIndex++)
  {
    auto *layerElement = xmlDocument.NewElement("Layer");
    layerElement->SetAttribute("index", layerIndex);
    layerElement->SetAttribute("labels", inputImage->GetNumberOfLabels(layerIndex));
    rootElement->InsertEndChild(layerElement);

    for (unsigned int labelIndex = 0; labelIndex < inputImage->GetNumberOfLabels(layerIndex); labelIndex++)
      layerElement->InsertEndChild(MultiLabelIOHelper::GetLabelAsXMLElement(xmlDocument, inputImage->GetLabel(labelIndex, layerIndex)));
  }

  return tinyxml2::XML_SUCCESS == xmlDocument.SaveFile(filename.c_str());
}

bool mitk::MultiLabelIOHelper::LoadLabelSetImagePreset(const std::string &presetFilename,
                                                     mitk::LabelSetImage *inputImage)
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

  auto activeLayerBackup = inputImage->GetActiveLayer();

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

    if (nullptr == inputImage->GetLabelSet(layerIndex))
    {
      inputImage->AddLayer();
    }
    else
    {
      inputImage->SetActiveLayer(layerIndex);
    }

    auto *labelElement = layerElement->FirstChildElement("Label");

    if (nullptr == labelElement)
      continue;

    for (int labelIndex = 0; labelIndex < numberOfLabels; labelIndex++)
    {
      auto label = mitk::MultiLabelIOHelper::LoadLabelFromXMLDocument(labelElement);
      const auto labelValue = label->GetValue();

      if (LabelSetImage::UnlabeledValue != labelValue)
      {
        auto* labelSet = inputImage->GetLabelSet(layerIndex);
        auto* alreadyExistingLabel = labelSet->GetLabel(labelValue);

        if (nullptr != alreadyExistingLabel)
        {
          // Override existing label with label from preset
          alreadyExistingLabel->ConcatenatePropertyList(label);
          labelSet->UpdateLookupTable(labelValue);
        }
        else
        {
          labelSet->AddLabel(label);
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

  inputImage->SetActiveLayer(activeLayerBackup);

  return true;
}

tinyxml2::XMLElement *mitk::MultiLabelIOHelper::GetLabelAsXMLElement(tinyxml2::XMLDocument &doc, Label *label)
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

nlohmann::json mitk::MultiLabelIOHelper::SerializeMultLabelGroupsToJSON(const mitk::LabelSetImage* inputImage)
{
  if (nullptr == inputImage)
  {
    mitkThrow() << "Invalid call of SerializeMultLabelGroupsToJSON. Passed image pointer is null.";
  }

  nlohmann::json result;

  for (LabelSetImage::GroupIndexType i = 0; i < inputImage->GetNumberOfLayers(); i++)
  {
    nlohmann::json jgroup;
    nlohmann::json jlabels;

    for (const auto& label : inputImage->GetLabelsInGroup(i))
    {
      jlabels.emplace_back(SerializeLabelToJSON(label));
    }
    jgroup["labels"] = jlabels;
    result.emplace_back(jgroup);
  }
  return result;
};

std::vector<mitk::LabelSet::Pointer> mitk::MultiLabelIOHelper::DeserializeMultiLabelGroupsFromJSON(const nlohmann::json& listOfLabelSets)
{
  std::vector<LabelSet::Pointer> result;

  for (const auto& jlabelset : listOfLabelSets)
  {
    LabelSet::Pointer labelSet = LabelSet::New();
    if (jlabelset.find("labels") != jlabelset.end())
    {
      auto jlabels = jlabelset["labels"];

      for (const auto& jlabel : jlabels)
      {
        auto label = DeserializeLabelFromJSON(jlabel);
        labelSet->AddLabel(label, false);
      }
    }
    result.emplace_back(labelSet);
  }

  return result;
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

  nlohmann::json jcolor;
  jcolor["type"] = "ColorProperty";
  jcolor["value"] = {label->GetColor().GetRed(), label->GetColor().GetGreen(), label->GetColor().GetBlue() };
  j["color"] = jcolor;

  j["locked"] = label->GetLocked();
  j["opacity"] = label->GetOpacity();
  j["visible"] = label->GetVisible();
  return j;
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

mitk::Label::Pointer mitk::MultiLabelIOHelper::DeserializeLabelFromJSON(const nlohmann::json& labelJson)
{
  Label::Pointer resultLabel = Label::New();

  std::string name = "Unkown label name";
  GetValueFromJson(labelJson, "name", name);
  resultLabel->SetName(name);

  Label::PixelType value = 1;
  GetValueFromJson(labelJson, "value", value);
  resultLabel->SetValue(value);

  if (labelJson.find("color") != labelJson.end())
  {
    auto jcolor = labelJson["color"]["value"];
    Color color;
    color.SetRed(jcolor[0].get<float>());
    color.SetGreen(jcolor[1].get<float>());
    color.SetBlue(jcolor[2].get<float>());

    resultLabel->SetColor(color);
  }

  bool locked = false;
  if (GetValueFromJson(labelJson, "locked", locked))
    resultLabel->SetLocked(locked);

  float opacity = 1.;
  if (GetValueFromJson(labelJson, "opacity", opacity))
    resultLabel->SetOpacity(opacity);


  bool visible = true;
  if (GetValueFromJson(labelJson, "visible", visible))
    resultLabel->SetVisible(visible);

  return resultLabel;
}
