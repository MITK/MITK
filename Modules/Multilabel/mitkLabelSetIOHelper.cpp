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

  auto *presetXmlDoc = new TiXmlDocument();

  auto *decl = new TiXmlDeclaration("1.0", "", "");
  presetXmlDoc->LinkEndChild(decl);

  auto *presetElement = new TiXmlElement("LabelSetImagePreset");
  presetElement->SetAttribute("layers", inputImage->GetNumberOfLayers());

  presetXmlDoc->LinkEndChild(presetElement);

  for (unsigned int layerIdx = 0; layerIdx < inputImage->GetNumberOfLayers(); layerIdx++)
  {
    auto *layerElement = new TiXmlElement("Layer");
    layerElement->SetAttribute("index", layerIdx);
    layerElement->SetAttribute("labels", inputImage->GetNumberOfLabels(layerIdx));

    presetElement->LinkEndChild(layerElement);

    for (unsigned int labelIdx = 0; labelIdx < inputImage->GetNumberOfLabels(layerIdx); labelIdx++)
    {
      TiXmlElement *labelAsXml = LabelSetIOHelper::GetLabelAsTiXmlElement(inputImage->GetLabel(labelIdx, layerIdx));
      layerElement->LinkEndChild(labelAsXml);
    }
  }

  bool wasSaved = presetXmlDoc->SaveFile(filename);
  delete presetXmlDoc;

  return wasSaved;
}

void mitk::LabelSetIOHelper::LoadLabelSetImagePreset(const std::string &presetFilename,
                                                     mitk::LabelSetImage::Pointer &inputImage)
{
  const auto filename = EnsureExtension(presetFilename);

  std::unique_ptr<TiXmlDocument> presetXmlDoc(new TiXmlDocument());

  bool ok = presetXmlDoc->LoadFile(filename);
  if (!ok)
    return;

  TiXmlElement *presetElem = presetXmlDoc->FirstChildElement("LabelSetImagePreset");
  if (!presetElem)
  {
    MITK_INFO << "No valid preset XML";
    return;
  }

  int numberOfLayers;
  presetElem->QueryIntAttribute("layers", &numberOfLayers);

  for (int i = 0; i < numberOfLayers; i++)
  {
    TiXmlElement *layerElem = presetElem->FirstChildElement("Layer");
    int numberOfLabels;
    layerElem->QueryIntAttribute("labels", &numberOfLabels);

    if (inputImage->GetLabelSet(i) == nullptr)
      inputImage->AddLayer();

    TiXmlElement *labelElement = layerElem->FirstChildElement("Label");
    if (labelElement == nullptr)
      break;
    for (int j = 0; j < numberOfLabels; j++)
    {
      mitk::Label::Pointer label = mitk::LabelSetIOHelper::LoadLabelFromTiXmlDocument(labelElement);

      if (label->GetValue() == 0)
        inputImage->SetExteriorLabel(label);
      else
        inputImage->GetLabelSet()->AddLabel(label);

      labelElement = labelElement->NextSiblingElement("Label");
      if (labelElement == nullptr)
        break;
    }
  }
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
