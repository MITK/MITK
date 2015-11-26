/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkLabelSetIOHelper.h"

#include "mitkLabelSetImage.h"
#include <mitkBasePropertySerializer.h>
#include <tinyxml.h>


bool mitk::LabelSetIOHelper::SaveLabelSetImagePreset(std::string &presetFilename, mitk::LabelSetImage::Pointer &inputImage)
{
  if (presetFilename.find(".lsetp") == std::string::npos)
  {
    presetFilename.append(".lsetp");
  }

  TiXmlDocument * presetXmlDoc =  new TiXmlDocument();

  TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "", "" );
  presetXmlDoc->LinkEndChild( decl );

  TiXmlElement * presetElement = new TiXmlElement("LabelSetImagePreset");
  presetElement->SetAttribute("layers", inputImage->GetNumberOfLayers());

  presetXmlDoc->LinkEndChild(presetElement);

  for (unsigned int layerIdx=0; layerIdx<inputImage->GetNumberOfLayers(); layerIdx++)
  {
    TiXmlElement * layerElement = new TiXmlElement("Layer");
    layerElement->SetAttribute("index", layerIdx);
    layerElement->SetAttribute("labels", inputImage->GetNumberOfLabels(layerIdx));

    presetElement->LinkEndChild(layerElement);

    for (unsigned int labelIdx=0; labelIdx<inputImage->GetNumberOfLabels(layerIdx); labelIdx++)
    {
      TiXmlElement * labelAsXml = LabelSetIOHelper::GetLabelAsTiXmlElement(inputImage->GetLabel(labelIdx,layerIdx));
      layerElement->LinkEndChild(labelAsXml);
    }
  }

  bool wasSaved = presetXmlDoc->SaveFile(presetFilename);
  delete presetXmlDoc;

  return wasSaved;
}

void mitk::LabelSetIOHelper::LoadLabelSetImagePreset(std::string &presetFilename, mitk::LabelSetImage::Pointer &inputImage)
{
  if (presetFilename.find(".lsetp") == std::string::npos)
  {
    presetFilename.append(".lsetp");
  }

  std::unique_ptr<TiXmlDocument> presetXmlDoc(new TiXmlDocument());

  bool ok = presetXmlDoc->LoadFile(presetFilename);
  if ( !ok )
    return;

  TiXmlElement * presetElem = presetXmlDoc->FirstChildElement("LabelSetImagePreset");
  if(!presetElem)
  {
    MITK_INFO << "No valid preset XML";
    return;
  }

  int numberOfLayers;
  presetElem->QueryIntAttribute("layers", &numberOfLayers);

  for(int i = 0 ; i < numberOfLayers; i++)
  {
    TiXmlElement * layerElem = presetElem->FirstChildElement("Layer");
    int numberOfLabels;
    layerElem->QueryIntAttribute("labels", &numberOfLabels);

    if(inputImage->GetLabelSet(i) == NULL) inputImage->AddLayer();

    TiXmlElement * labelElement = layerElem->FirstChildElement("Label");
    if(labelElement == NULL) break;
    for(int j = 0 ; j < numberOfLabels; j++)
    {

      mitk::Label::Pointer label = mitk::LabelSetIOHelper::LoadLabelFromTiXmlDocument(labelElement);

      if(label->GetValue() == 0)
        inputImage->SetExteriorLabel(label);
      else
        inputImage->GetLabelSet()->AddLabel(label);

      labelElement = labelElement->NextSiblingElement("Label");
      if(labelElement == NULL) break;
    }
  }
}

TiXmlElement* mitk::LabelSetIOHelper::GetLabelAsTiXmlElement(Label *label)
{
  TiXmlElement* labelElem = new TiXmlElement("Label");

  // add XML contents
  const PropertyList::PropertyMap* propmap = label->GetMap();
  for ( PropertyList::PropertyMap::const_iterator iter = propmap->begin();
        iter != propmap->end();
        ++iter )
  {
    std::string key = iter->first;
    const BaseProperty* property = iter->second;
    TiXmlElement* element = PropertyToXmlElem( key, property );
    if (element)
     labelElem->LinkEndChild( element );

  }
  return labelElem;
}

mitk::Label::Pointer mitk::LabelSetIOHelper::LoadLabelFromTiXmlDocument(TiXmlElement *labelElem)
{
  // reread
  TiXmlElement * propElem = labelElem->FirstChildElement("property");

  std::string name;
  mitk::BaseProperty::Pointer prop;

  mitk::Label::Pointer label = mitk::Label::New();
  while(propElem)
  {
    LabelSetIOHelper::PropertyFromXmlElem( name, prop, propElem );
    label->SetProperty( name, prop );
    propElem = propElem->NextSiblingElement( "property" );
  }

  return label.GetPointer();
}

TiXmlElement* mitk::LabelSetIOHelper::PropertyToXmlElem( const std::string& key, const BaseProperty* property )
{
  TiXmlElement* keyelement = new TiXmlElement("property");
  keyelement->SetAttribute("key", key);
  keyelement->SetAttribute("type", property->GetNameOfClass());

  // construct name of serializer class
  std::string serializername(property->GetNameOfClass());
  serializername += "Serializer";

  std::list<itk::LightObject::Pointer> allSerializers = itk::ObjectFactoryBase::CreateAllInstance(serializername.c_str());
  if (allSerializers.size() < 1)
    MITK_ERROR << "No serializer found for " << property->GetNameOfClass() << ". Skipping object";

  if (allSerializers.size() > 1)
    MITK_WARN << "Multiple serializers found for " << property->GetNameOfClass() << "Using arbitrarily the first one.";

  for ( std::list<itk::LightObject::Pointer>::iterator iter = allSerializers.begin();
        iter != allSerializers.end();
        ++iter )
  {
    if (BasePropertySerializer* serializer = dynamic_cast<BasePropertySerializer*>( iter->GetPointer() ) )
    {
      serializer->SetProperty(property);
      try
      {
        TiXmlElement* valueelement = serializer->Serialize();
        if (valueelement)
          keyelement->LinkEndChild( valueelement );
      }
      catch (std::exception& e)
      {
        MITK_ERROR << "Serializer " << serializer->GetNameOfClass() << " failed: " << e.what();
      }
      break;
    }
  }
  return keyelement;
}

bool mitk::LabelSetIOHelper::PropertyFromXmlElem(std::string& key, mitk::BaseProperty::Pointer& prop, TiXmlElement* elem)
{
  std::string type;
  elem->QueryStringAttribute("type", &type);
  elem->QueryStringAttribute("key", &key);

  // construct name of serializer class
  std::string serializername(type);
  serializername += "Serializer";

  std::list<itk::LightObject::Pointer> allSerializers = itk::ObjectFactoryBase::CreateAllInstance(serializername.c_str());
  if (allSerializers.size() < 1)
    MITK_ERROR << "No serializer found for " << type << ". Skipping object";

  if (allSerializers.size() > 1)
    MITK_WARN << "Multiple deserializers found for " << type << "Using arbitrarily the first one.";

  for ( std::list<itk::LightObject::Pointer>::iterator iter = allSerializers.begin();
        iter != allSerializers.end();
        ++iter )
  {
    if (BasePropertySerializer* serializer = dynamic_cast<BasePropertySerializer*>( iter->GetPointer() ) )
    {
      try
      {
        prop = serializer->Deserialize(elem->FirstChildElement());
      }
      catch (std::exception& e)
      {
        MITK_ERROR << "Deserializer " << serializer->GetNameOfClass() << " failed: " << e.what();
        return false;
      }
      break;
    }
  }
  if(prop.IsNull())
    return false;
  return true;
}
