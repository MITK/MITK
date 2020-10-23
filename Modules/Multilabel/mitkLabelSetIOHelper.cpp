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
#include <tinyxml.h>

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

  TiXmlDocument xmlDocument;
  xmlDocument.LinkEndChild(new TiXmlDeclaration("1.0", "", ""));

  auto *rootElement = new TiXmlElement("LabelSetImagePreset");
  rootElement->SetAttribute("layers", inputImage->GetNumberOfLayers());
  xmlDocument.LinkEndChild(rootElement);

  for (unsigned int layerIndex = 0; layerIndex < inputImage->GetNumberOfLayers(); layerIndex++)
  {
    auto *layerElement = new TiXmlElement("Layer");
    layerElement->SetAttribute("index", layerIndex);
    layerElement->SetAttribute("labels", inputImage->GetNumberOfLabels(layerIndex));
    rootElement->LinkEndChild(layerElement);

    for (unsigned int labelIndex = 0; labelIndex < inputImage->GetNumberOfLabels(layerIndex); labelIndex++)
      layerElement->LinkEndChild(LabelSetIOHelper::GetLabelAsTiXmlElement(inputImage->GetLabel(labelIndex, layerIndex)));
  }

  return xmlDocument.SaveFile(filename);
}

void mitk::LabelSetIOHelper::LoadLabelSetImagePreset(const std::string &presetFilename,
                                                     mitk::LabelSetImage::Pointer &inputImage)
{
  if (inputImage.IsNull())
    return;

  const auto filename = EnsureExtension(presetFilename);

  TiXmlDocument xmlDocument;

  if(!xmlDocument.LoadFile(filename))
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
      auto label = mitk::LabelSetIOHelper::LoadLabelFromTiXmlDocument(labelElement);
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

TiXmlElement *mitk::LabelSetIOHelper::GetLabelAsTiXmlElement(Label *label)
{
  auto *labelElem = new TiXmlElement("Label");

  // add XML contents
  const PropertyList::PropertyMap *propmap = label->GetMap();
  for (auto iter = propmap->begin(); iter != propmap->end(); ++iter)
  {
    std::string key = iter->first;
    const BaseProperty *property = iter->second;
    TiXmlElement *element = PropertyToXmlElem(key, property);
    if (element)
      labelElem->LinkEndChild(element);
  }
  return labelElem;
}

mitk::Label::Pointer mitk::LabelSetIOHelper::LoadLabelFromTiXmlDocument(TiXmlElement *labelElem)
{
  // reread
  TiXmlElement *propElem = labelElem->FirstChildElement("property");

  std::string name;
  mitk::BaseProperty::Pointer prop;

  mitk::Label::Pointer label = mitk::Label::New();
  while (propElem)
  {
    LabelSetIOHelper::PropertyFromXmlElem(name, prop, propElem);
    label->SetProperty(name, prop);
    propElem = propElem->NextSiblingElement("property");
  }

  return label.GetPointer();
}

TiXmlElement *mitk::LabelSetIOHelper::PropertyToXmlElem(const std::string &key, const BaseProperty *property)
{
  auto *keyelement = new TiXmlElement("property");
  keyelement->SetAttribute("key", key);
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
        TiXmlElement *valueelement = serializer->Serialize();
        if (valueelement)
          keyelement->LinkEndChild(valueelement);
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

bool mitk::LabelSetIOHelper::PropertyFromXmlElem(std::string &key,
                                                 mitk::BaseProperty::Pointer &prop,
                                                 TiXmlElement *elem)
{
  std::string type;
  elem->QueryStringAttribute("type", &type);
  elem->QueryStringAttribute("key", &key);

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
