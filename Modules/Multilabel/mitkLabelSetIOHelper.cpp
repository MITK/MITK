/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkLabelSetIOHelper.h"

#include "mitkLabelSetImage.h"
#include <mitkBasePropertySerializer.h>

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

bool mitk::LabelSetIOHelper::SaveLabelSetImagePreset(const std::string &presetFilename,
                                                     mitk::LabelSetImage::Pointer &inputImage)
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
      layerElement->InsertEndChild(LabelSetIOHelper::GetLabelAsXMLElement(xmlDocument, inputImage->GetLabel(labelIndex, layerIndex)));
  }

  return tinyxml2::XML_SUCCESS == xmlDocument.SaveFile(filename.c_str());
}

void mitk::LabelSetIOHelper::LoadLabelSetImagePreset(const std::string &presetFilename,
                                                     mitk::LabelSetImage::Pointer &inputImage)
{
  if (inputImage.IsNull())
    return;

  const auto filename = EnsureExtension(presetFilename);

  tinyxml2::XMLDocument xmlDocument;

  if(tinyxml2::XML_SUCCESS != xmlDocument.LoadFile(filename.c_str()))
    return;

  auto *rootElement = xmlDocument.FirstChildElement("LabelSetImagePreset");

  if (nullptr == rootElement)
  {
    MITK_WARN << "Not a valid LabelSet preset";
    return;
  }

  auto activeLayerBackup = inputImage->GetActiveLayer();

  int numberOfLayers = 0;
  rootElement->QueryIntAttribute("layers", &numberOfLayers);

  auto* layerElement = rootElement->FirstChildElement("Layer");

  if (nullptr == layerElement)
  {
    MITK_WARN << "LabelSet preset does not contain any layers";
    return;
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
      auto label = mitk::LabelSetIOHelper::LoadLabelFromXMLDocument(labelElement);
      const auto labelValue = label->GetValue();

      if (0 != labelValue)
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
}

tinyxml2::XMLElement *mitk::LabelSetIOHelper::GetLabelAsXMLElement(tinyxml2::XMLDocument &doc, Label *label)
{
  auto *labelElem = doc.NewElement("Label");

  // add XML contents
  const PropertyList::PropertyMap *propmap = label->GetMap();
  for (auto iter = propmap->begin(); iter != propmap->end(); ++iter)
  {
    std::string key = iter->first;
    const BaseProperty *property = iter->second;
    auto *element = PropertyToXMLElement(doc, key, property);
    if (element)
      labelElem->InsertEndChild(element);
  }
  return labelElem;
}

mitk::Label::Pointer mitk::LabelSetIOHelper::LoadLabelFromXMLDocument(const tinyxml2::XMLElement *labelElem)
{
  // reread
  auto *propElem = labelElem->FirstChildElement("property");

  std::string name;
  mitk::BaseProperty::Pointer prop;

  mitk::Label::Pointer label = mitk::Label::New();
  while (propElem)
  {
    LabelSetIOHelper::PropertyFromXMLElement(name, prop, propElem);
    label->SetProperty(name, prop);
    propElem = propElem->NextSiblingElement("property");
  }

  return label.GetPointer();
}

tinyxml2::XMLElement *mitk::LabelSetIOHelper::PropertyToXMLElement(tinyxml2::XMLDocument &doc, const std::string &key, const BaseProperty *property)
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

bool mitk::LabelSetIOHelper::PropertyFromXMLElement(std::string &key,
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
