/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkMultiLabelIOHelper.h>

#include <mitkLabelSetImage.h>
#include <mitkBasePropertySerializer.h>
#include <mitkDICOMSegmentationConstants.h>
#include <mitkProperties.h>
#include <mitkStringProperty.h>

#include <itkMetaDataDictionary.h>
#include <itkMetaDataObject.h>

#include <tinyxml2.h>

#include <string_view>

namespace
{
  /**
   * MITK versions before 2025.12 stamped every label with copies of its number and name, a
   * SEMIAUTOMATIC algorithm type and codes looked up by label name, under the DICOM tag paths
   * of the segment sequence. The DICOM SEG reader of that time wrote the file's algorithm type
   * and codes to the same keys, though only for the first segment. The label attributes have
   * their own keys now, and most of these values are stamps rather than information, so they
   * are dropped instead of translated; the codes imported that way are lost with them.
   */
  const std::string& GetOutdatedLabelPropertyPrefix()
  {
    static const std::string prefix =
      mitk::DICOMTagPathToPropertyName(mitk::DICOMSegmentationConstants::SEGMENT_SEQUENCE_PATH()) + ".";
    return prefix;
  }

  bool IsOutdatedLabelProperty(const std::string& key)
  {
    const auto& prefix = GetOutdatedLabelPropertyPrefix();
    return 0 == key.compare(0, prefix.size(), prefix);
  }

  bool LoadLegacyLabelSetImagePreset(const std::string& presetFilename,
    mitk::MultiLabelSegmentation* inputImage)
  {
    if (nullptr == inputImage)
      return false;

    tinyxml2::XMLDocument xmlDocument;

    if (tinyxml2::XML_SUCCESS != xmlDocument.LoadFile(presetFilename.c_str()))
    {
      MITK_WARN << "Label set preset file \"" << presetFilename << "\" does not exist or cannot be opened";
      return false;
    }

    auto* rootElement = xmlDocument.FirstChildElement("LabelSetImagePreset");

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

      auto* labelElement = layerElement->FirstChildElement("Label");

      if (nullptr == labelElement)
        continue;

      for (int labelIndex = 0; labelIndex < numberOfLabels; labelIndex++)
      {
        auto label = mitk::MultiLabelIOHelper::LoadLabelFromXMLDocument(labelElement);
        const auto labelValue = label->GetValue();

        if (mitk::MultiLabelSegmentation::UNLABELED_VALUE != labelValue)
        {
          if (inputImage->ExistLabel(labelValue, layerIndex))
          {
            // Override existing label with label from preset
            auto alreadyExistingLabel = inputImage->GetLabel(labelValue);
            alreadyExistingLabel->ConcatenatePropertyList(label);
            inputImage->UpdateLookupTable(labelValue);
          }
          else
          {
            if (inputImage->ExistLabel(labelValue))
            { //label is the wrong group so we need to remove it there before adding it in its new form.
              inputImage->RemoveLabel(labelValue);
            }
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
    else if (inputImage->GetTotalNumberOfLabels() > 0)
    {
      inputImage->SetActiveLabel(inputImage->GetAllLabelValues().front());
    }

    return true;
  }


  bool LoadNewJSONPreset(const std::string& presetFilename,
    mitk::MultiLabelSegmentation* inputImage)
  {
    if (nullptr == inputImage)
      return false;

    std::ifstream input(presetFilename);
    if (!input.is_open())
    {
      return false;
    }

    nlohmann::json fileContent;
    try
    {
      input >> fileContent;
    }
    catch (const nlohmann::json::parse_error& e)
    {
      mitkThrow() << "Cannot read data due to parsing error. Parse error: " << e.what() << '\n';
    }

    //check version
    int version = 0;

    if (!mitk::MultiLabelIOHelper::GetValueFromJson<int>(fileContent, "version", version))
    {
      MITK_WARN << "Preset has unknown version. Assuming that it can be read. Result might be invalid.";
    }
    else
    {
      const int MULTILABEL_SEGMENTATION_VERSION_VALUE = 4;
      if (version > MULTILABEL_SEGMENTATION_VERSION_VALUE)
      {
        mitkThrow() << "The preset you are trying to load has an unsupported version. "
            << "This software is too old to ensure correct loading. "
            << "Please use a compatible version of MITK or save the data in another format. "
            << "Data version: " << version 
            << "; supported versions up to: " << MULTILABEL_SEGMENTATION_VERSION_VALUE;

      }
    }

    auto groupInfos = mitk::MultiLabelIOHelper::DeserializeMultiLabelGroupsFromJSON(fileContent["groups"]);

    mitk::MultiLabelSegmentation::GroupIndexType groupIndex = 0;
    for (const auto& groupInfo : groupInfos)
    {
      auto cleanedLabels = mitk::MultiLabelIOHelper::CloneLabelsWithoutMetaProperties(groupInfo.labels);

      if (inputImage->ExistGroup(groupIndex))
      { //group exists so update name and labels
        inputImage->SetGroupName(groupIndex, groupInfo.name);

        for (auto label : cleanedLabels)
        {
          if (inputImage->ExistLabel(label->GetValue(), groupIndex))
          {
            // Override existing label with label from preset
            auto alreadyExistingLabel = inputImage->GetLabel(label->GetValue());
            alreadyExistingLabel->Update(label);
            inputImage->UpdateLookupTable(label->GetValue());
          }
          else
          {
            if (inputImage->ExistLabel(label->GetValue()))
            { //label is the wrong group so we need to remove it there before adding it in its new form.
              inputImage->RemoveLabel(label->GetValue());
            }
            const bool adaptValue = label->GetValue() == mitk::Label::UNLABELED_VALUE ? true : false;
            inputImage->AddLabel(label, groupIndex, false, adaptValue);
          }
        }
      }
      else
      { //add new group
        inputImage->AddGroup();
        inputImage->SetGroupName(groupIndex, groupInfo.name);
        for (auto label : cleanedLabels)
        {
          const bool adaptValue = label->GetValue() == mitk::Label::UNLABELED_VALUE ? true : false;
          inputImage->AddLabel(label, groupIndex, false, adaptValue);
        }
      }

      groupIndex++;
    }

    return true;
  }
}

bool mitk::MultiLabelIOHelper::SaveMultiLabelSegmentationPreset(const std::string &presetFilename,
                                                     const mitk::MultiLabelSegmentation* input)
{
  if (nullptr == input)
    return false;

  int MULTILABEL_SEGMENTATION_VERSION_VALUE = 4;
  nlohmann::json stackContent;
  stackContent["version"] = MULTILABEL_SEGMENTATION_VERSION_VALUE;
  stackContent["type"] = "org.mitk.multilabel.segmentation.preset";
  stackContent["groups"] = MultiLabelIOHelper::SerializeMultLabelGroupsToJSON(input);

  std::ofstream file(presetFilename);
  if (file.is_open())
  {
    file << std::setw(4) << stackContent << std::endl;
  }
  else
  {
    mitkThrow() << "Cannot write meta data. Cannot open file: " << presetFilename;
  }

  return true;
}

bool mitk::MultiLabelIOHelper::LoadMultiLabelSegmentationPreset(const std::string &presetFilename,
                                                     mitk::MultiLabelSegmentation *inputSegmentation)
{
  if (nullptr == inputSegmentation)
    return false;

  //first try new format
  bool result = false;
  try
  {
    result = LoadNewJSONPreset(presetFilename, inputSegmentation);
  }
  catch (Exception& e)
  {
    MITK_INFO << "Loading of multi label preset as JSON failed. Reason: " <<e.what();
  }

  if (!result)
  { // try to load with legacy format
    MITK_INFO << "Try to load as legacy xml preset.";
    try
    {
      return LoadLegacyLabelSetImagePreset(presetFilename, inputSegmentation);
    }
    catch (const Exception& e)
    {
      MITK_WARN << "Cannot load label set preset \"" << presetFilename << "\": " << e.what();
      return false;
    }
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

mitk::Label::Pointer mitk::MultiLabelIOHelper::LoadLabelFromXMLDocument(const tinyxml2::XMLElement *labelElem,
  bool* hadOutdatedProperties)
{
  if (nullptr != hadOutdatedProperties)
    *hadOutdatedProperties = false;

  // reread
  auto *propElem = labelElem->FirstChildElement("property");

  mitk::Label::Pointer label = mitk::Label::New();
  while (propElem)
  {
    // Checked before deserializing, so that a property that is dropped anyway cannot fail the read.
    const char* key = propElem->Attribute("key");
    if (nullptr != key && IsOutdatedLabelProperty(key))
    {
      if (nullptr != hadOutdatedProperties)
        *hadOutdatedProperties = true;
      propElem = propElem->NextSiblingElement("property");
      continue;
    }

    // The layer of the XML format is the group of the label now. Kept as a property, every later
    // save would write it back, and after splitting the layers it would be stale.
    if (nullptr != key && std::string_view(key) == "layer")
    {
      propElem = propElem->NextSiblingElement("property");
      continue;
    }

    std::string name;
    mitk::BaseProperty::Pointer prop;
    if (!MultiLabelIOHelper::PropertyFromXMLElement(name, prop, propElem))
      mitkThrow() << "Cannot load the label property '" << name << "' from XML: its value cannot be deserialized.";

    if (name == "value")
    {
      auto castedProp = dynamic_cast<mitk::UShortProperty*>(prop.GetPointer());
      if (castedProp != nullptr)
      {
        label->SetValue(castedProp->GetValue());
      }
      else
        mitkThrow() << "Cannot load legacy label information from xml. Value property is encoded with the wrong property type. Used property type: " << *prop.GetPointer();
    }
    else
    {
      label->SetProperty(name, prop);
    }
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
  return "";
}

mitk::MultiLabelIOHelper::LabelGroupMetaData::LabelGroupMetaData() : properties(PropertyList::New())
{};

mitk::MultiLabelIOHelper::LabelGroupMetaData::LabelGroupMetaData(const std::string& groupName, const mitk::LabelVector& groupLabels)
  : name(groupName), labels(groupLabels), properties(PropertyList::New())
{};

mitk::MultiLabelIOHelper::LabelGroupMetaData::LabelGroupMetaData(const std::string& groupName, const mitk::LabelVector& groupLabels, PropertyList* groupProperties)
  : name(groupName), labels(groupLabels), properties(groupProperties)
{};

nlohmann::json mitk::MultiLabelIOHelper::SerializeMultLabelGroupsToJSON(const mitk::MultiLabelSegmentation* inputImage,
  GroupFileNameCallback groupFileNameCallback, LabelFileNameCallback labelFileNameCallback,
  LabelFileValueCallback labelFileValueCallback)
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
      auto jLabel = SerializeLabelToJSON(label);
      if (nullptr != labelFileNameCallback)
      {
        auto labelFile = labelFileNameCallback(inputImage, label->GetValue());
        jLabel["_file"] = labelFile;
      }
      if (nullptr != labelFileValueCallback)
      {
        auto labelValue = labelFileValueCallback(inputImage, label->GetValue());
        if (labelValue != label->GetValue())
        {
          jLabel["_file_value"] = labelValue;
        }
      }
      jlabels.emplace_back(jLabel);
    }

    if (!jlabels.empty())
    {
      jgroup["labels"] = jlabels;
    }

    const auto& name = inputImage->GetGroupName(i);
    if (!name.empty())
    {
      jgroup["name"] = name;
    }

    if (nullptr != groupFileNameCallback)
    {
      auto groupFile = groupFileNameCallback(inputImage, i);
      jgroup["_file"] = groupFile;
    }

    result.emplace_back(jgroup);
  }
  return result;
};

std::vector<mitk::MultiLabelIOHelper::LabelGroupMetaData> mitk::MultiLabelIOHelper::DeserializeMultiLabelGroupsFromJSON(const nlohmann::json& listOfLabelGroups,
  std::vector<Label::PixelType>* labelsWithOutdatedProperties)
{
  if (nullptr != labelsWithOutdatedProperties)
    labelsWithOutdatedProperties->clear();

  std::vector<LabelGroupMetaData> result;

  for (const auto& jLabelGroup : listOfLabelGroups)
  {
    LabelVector labels;
    std::string name;
    auto customProps = PropertyList::New();

    for (const auto& [key, jValue] : jLabelGroup.items())
    {
      if (key=="labels")
      {
        auto jlabels = jLabelGroup["labels"];

        for (const auto& jlabel : jlabels)
        {
          bool hadOutdatedProperties = false;
          auto label = DeserializeLabelFromJSON(jlabel, &hadOutdatedProperties);
          if (hadOutdatedProperties && nullptr != labelsWithOutdatedProperties)
            labelsWithOutdatedProperties->push_back(label->GetValue());
          labels.push_back(label);
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
          MITK_ERROR << "Unable to read group key from JSON. Value has wrong type. Failed key: " << key << "; invalid value: " << jValue.dump();
          mitkThrow() << "Unable to read group key from JSON. Value has wrong type. Failed key: " << key << "; invalid value: " << jValue.dump();
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
    "tracking_id", "tracking_uid", "description",
    //the following keys will be ignored on purpose
    "center.coordinates", "center.index"
  };

  for (const auto& [key, property] : *propMap)
  {
    // Skip reserved keys
    if (reservedKeys.find(key) != reservedKeys.end())
      continue;

    // Readers ignore these, so writing them would only defer the loss; such a property was set
    // deliberately, since loaded labels never carry one.
    if (IsOutdatedLabelProperty(key))
    {
      MITK_WARN << "The property \"" << key << "\" of label " << label->GetValue() << " is not written: readers ignore"
                << " label properties under \"" << GetOutdatedLabelPropertyPrefix() << "\" as outdated.";
      continue;
    }

    // Try to serialize the property to JSON
    j[key] = mitk::MultiLabelIOHelper::SerializeLabelPropertyToJSON(property);
  }
}

nlohmann::json mitk::MultiLabelIOHelper::SerializeLabelPropertyToJSON(const BaseProperty* property)
{
  // Delegate to the generic PropertyJsonSerialization utility in MitkCore
  return ConvertPropertyToSelfContainedJson(property);
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
  // Only emit tracking keys when the value is non-empty. Has*() on the
  // loaded label then reflects "source carried a real value", uniformly
  // across DICOM SEG and native JSON. mitk::Label has no UID auto-
  // generation; the older empty-string-stamp pattern guarded against a
  // behaviour that does not exist.
  if (!label->GetTrackingID().empty())
    j["tracking_id"] = label->GetTrackingID();
  if (!label->GetTrackingUID().empty())
    j["tracking_uid"] = label->GetTrackingUID();
  if (!label->GetDescription().empty())
    j["description"] = label->GetDescription();

  // Serialize any other custom properties
  SerializeLabelCustomPropertiesToJSON(label, j);

  return j;
};

namespace
{
  const std::map<std::string, std::string>& GetConvinienceNameLookup()
  {
    static const std::map<std::string, std::string> lookup = {
      // Mapping for the case that human defined jsons miss the index
        { mitk::LabelPropertyConstants::GetAnatomicRegionPropertyBaseName() + "." + mitk::LabelPropertyConstants::GetMeaningPropertySubName(),
        mitk::LabelPropertyConstants::GetAnatomicRegionPropertyBaseName() + ".[0]." + mitk::LabelPropertyConstants::GetMeaningPropertySubName()},
        { mitk::LabelPropertyConstants::GetAnatomicRegionPropertyBaseName() + "." + mitk::LabelPropertyConstants::GetSchemePropertySubName(),
        mitk::LabelPropertyConstants::GetAnatomicRegionPropertyBaseName() + ".[0]." + mitk::LabelPropertyConstants::GetSchemePropertySubName() },
        { mitk::LabelPropertyConstants::GetAnatomicRegionPropertyBaseName() + "." + mitk::LabelPropertyConstants::GetValuePropertySubName(),
        mitk::LabelPropertyConstants::GetAnatomicRegionPropertyBaseName() + ".[0]." + mitk::LabelPropertyConstants::GetValuePropertySubName() },
        { mitk::LabelPropertyConstants::GetPrimaryAnatomicStructurePropertyBaseName() + "." + mitk::LabelPropertyConstants::GetMeaningPropertySubName(),
        mitk::LabelPropertyConstants::GetPrimaryAnatomicStructurePropertyBaseName() + ".[0]." + mitk::LabelPropertyConstants::GetMeaningPropertySubName()},
        { mitk::LabelPropertyConstants::GetPrimaryAnatomicStructurePropertyBaseName() + "." + mitk::LabelPropertyConstants::GetSchemePropertySubName(),
        mitk::LabelPropertyConstants::GetPrimaryAnatomicStructurePropertyBaseName() + ".[0]." + mitk::LabelPropertyConstants::GetSchemePropertySubName() },
        { mitk::LabelPropertyConstants::GetPrimaryAnatomicStructurePropertyBaseName() + "." + mitk::LabelPropertyConstants::GetValuePropertySubName(),
        mitk::LabelPropertyConstants::GetPrimaryAnatomicStructurePropertyBaseName() + ".[0]." + mitk::LabelPropertyConstants::GetValuePropertySubName() }
    };
    return lookup;
  }
  std::string EnsureCorrectPropertyName(const std::string& externalName)
  {
    const auto& mapping = GetConvinienceNameLookup();
    auto finding = mapping.find(externalName);

    if (finding != mapping.end())
    {
      MITK_DEBUG << "Deserialized label used a simplified index-less name. Converted name into valid name. Original name: \"" << externalName << "\"; new name: \"" << finding->second << "\"";
      return finding->second;
    }

    return externalName;
  }
}

void mitk::MultiLabelIOHelper::WarnAboutOutdatedLabelProperties(const std::string& location, std::size_t numberOfLabels)
{
  if (0 == numberOfLabels)
    return;

  MITK_WARN << "The segmentation \"" << location << "\" carries outdated label properties (keys starting with \""
            << GetOutdatedLabelPropertyPrefix() << "\"; labels affected: " << numberOfLabels
            << "). They were ignored. Mostly they are defaults that MITK versions before 2025.12 stamped on"
               " every label, and that MITK 2025.12 to 2026.06 kept when saving the file again. If a version"
               " before 2025.12 imported the segmentation from DICOM SEG, they may hold codes of that file"
               " (segment category, type, modifier, anatomic region) that the label now lacks; to keep them,"
               " import the original DICOM SEG again, because saving this segmentation removes them for good."
               " Ignoring these properties is deprecated and may be removed in a future version.";
}

mitk::Label::Pointer mitk::MultiLabelIOHelper::DeserializeLabelFromJSON(const nlohmann::json& labelJson,
  bool* hadOutdatedProperties)
{
  if (nullptr != hadOutdatedProperties)
    *hadOutdatedProperties = false;

  Label::Pointer resultLabel = Label::New();

  for (const auto& [key, jValue] : labelJson.items())
  {
    if (IsOutdatedLabelProperty(key))
    {
      if (nullptr != hadOutdatedProperties)
        *hadOutdatedProperties = true;
      continue;
    }

    auto internalKey = EnsureCorrectPropertyName(key);

    if (internalKey == "name")
    {
      std::string name;
      if (GetValueFromJson(labelJson, "name", name))
        resultLabel->SetName(name);
    }
    else if (internalKey == "value")
    {
      Label::PixelType value = 1;
      if (GetValueFromJson(labelJson, "value", value))
        resultLabel->SetValue(value);
    }
    else if (internalKey == "color")
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
        if (jValue[0].is_number_float())
        { //assume color coded in float (0.0..1.0)
          color.SetRed(jValue[0].get<float>());
          color.SetGreen(jValue[1].get<float>());
          color.SetBlue(jValue[2].get<float>());
        }
        else
        { //assume color coded in int (0..255)
          color.SetRed(jValue[0].get<unsigned int>()/255.f);
          color.SetGreen(jValue[1].get<unsigned int>()/255.f);
          color.SetBlue(jValue[2].get<unsigned int>()/255.f);
        }
      }
      resultLabel->SetColor(color);
    }
    else if (internalKey == "locked")
    {
      bool locked = false;
      if (GetValueFromJson(labelJson, "locked", locked))
        resultLabel->SetLocked(locked);
    }
    else if (internalKey == "opacity")
    {
      float opacity = 1.;
      if (GetValueFromJson(labelJson, "opacity", opacity))
        resultLabel->SetOpacity(opacity);
    }
    else if (internalKey == "visible")
    {
      bool visible = true;
      if (GetValueFromJson(labelJson, "visible", visible))
        resultLabel->SetVisible(visible);
    }
    else if (internalKey == "tracking_uid")
    {
      std::string tracking_uid;
      // Skip the setter on empty values: legacy native JSON files stamped
      // empty strings to suppress a non-existent UID auto-generation. After
      // alignment with the DICOM SEG reader, HasTrackingUID() == true means
      // a real value was carried.
      if (GetValueFromJson(labelJson, "tracking_uid", tracking_uid) && !tracking_uid.empty())
        resultLabel->SetTrackingUID(tracking_uid);
    }
    else if (internalKey == "tracking_id")
    {
      std::string tracking_id;
      if (GetValueFromJson(labelJson, "tracking_id", tracking_id) && !tracking_id.empty())
        resultLabel->SetTrackingID(tracking_id);
    }
    else if (internalKey == "description")
    {
      std::string description;
      if (GetValueFromJson(labelJson, "description", description))
        resultLabel->SetDescription(description);
    }
    else
    { //unknown custom key that we just store as additional property
      // Delegate to PropertyJsonSerialization for property deserialization
      try
      {
        auto property = ConvertPropertyFromSelfContainedJson(jValue);
        resultLabel->SetProperty(internalKey, property);
      }
      catch (Exception& e)
      {
        MITK_ERROR << "Unable to read custom label property from JSON. Value has wrong type. Failed key: " << internalKey << "; invalid value: " << jValue.dump();
        mitkReThrow(e) << "Unable to read custom label property from JSON. Value has wrong type. Failed key: " << internalKey << "; invalid value: " << jValue.dump();
      }
      catch (const nlohmann::json::exception& e)
      {
        mitkThrow() << "Unable to read custom label property from JSON. Failed key: " << internalKey << "; invalid value: " << jValue.dump() << "; reason: " << e.what();
      }
    }
  }

  return resultLabel;
}

void mitk::MultiLabelIOHelper::RemoveMetaPropertiesFromLabel(Label* label)
{
  if (nullptr == label)
  {
    mitkThrow() << "Invalid call of RemoveMetaPropertiesFromLabel. Passed label pointer is null.";
  }
  auto labelKeys = label->GetPropertyKeys();
  for (const auto& key : labelKeys)
  {
    if (key.find('_') == 0)
    {
      label->RemoveProperty(key);
    }
  }
}

mitk::LabelVector mitk::MultiLabelIOHelper::CloneLabelsWithoutMetaProperties(const LabelVector& labels)
{
  mitk::MultiLabelSegmentation::LabelVectorType result;

  for (const auto& label : labels)
  {
    auto cleanedLabel = label->Clone();
    mitk::MultiLabelIOHelper::RemoveMetaPropertiesFromLabel(cleanedLabel);
    result.push_back(cleanedLabel);
  }

  return result;
}
