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


#include "mitkLabel.h"

#include "itkProcessObject.h"
#include <mitkProperties.h>
#include <itkCommand.h>
#include "tinyxml.h"

#include "mitkBasePropertySerializer.h"

#include <mitkStringProperty.h>


mitk::Label::Label():
  PropertyList()
{
  // initialize basic label properties
  SetLocked(true);
  SetVisible(true);
  SetOpacity(0.6);
  SetValue(-1);
  SetLayer(0);
  mitk::Point3D pnt;
  pnt.SetElement(0,0);
  pnt.SetElement(1,0);
  pnt.SetElement(2,0);
  SetCenterOfMassCoordinates(pnt);
  SetCenterOfMassIndex(pnt);

  SetName("NoName");

  mitk::Color col;
  col.Set(0,0,0);
  SetColor(col);
}

mitk::Label::Label(const Label& other)
  : PropertyList(other)
  // copyconstructer of property List handles the coping action
{
}

mitk::Label::~Label()
{

}

TiXmlDocument mitk::Label::GetAsTiXmlDocument() const
{
  TiXmlDocument document;
  TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" ); // TODO what to write here? encoding? etc....
  document.LinkEndChild( decl );

  TiXmlElement* version = new TiXmlElement("Version");
  version->SetAttribute("Writer",  __FILE__ );
  version->SetAttribute("Revision",  "$Revision: 17055 $" );
  version->SetAttribute("FileVersion",  1 );
  document.LinkEndChild(version);

  // add XML contents
  const PropertyList::PropertyMap* propmap = this->GetMap();
  for ( PropertyList::PropertyMap::const_iterator iter = propmap->begin();
        iter != propmap->end();
        ++iter )
  {
    std::string key = iter->first;
    const BaseProperty* property = iter->second;
    TiXmlElement* element = PropertyToXmlElem( key, property );
    if (element)
      document.LinkEndChild( element );
  }

  return document;
}

TiXmlElement* mitk::Label::PropertyToXmlElem( const std::string& key, const BaseProperty* property ) const
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

bool mitk::Label::LoadFromTiXmlDocument(TiXmlDocument * doc)
{
  // reread
  TiXmlHandle docHandle( doc );

  bool opRead = false;
  TiXmlElement * propElem = docHandle.FirstChildElement("Label").FirstChildElement("property").ToElement();

  std::string name;
  mitk::BaseProperty::Pointer prop;
  while(propElem)
  {
    opRead = this->PropertyFromXmlElem( name, prop, propElem );
    if(!opRead)
      break;
    this->SetProperty( name, prop );
    propElem = propElem->NextSiblingElement( "property" );
  }

  return opRead;
}

bool mitk::Label::PropertyFromXmlElem(std::string& key, mitk::BaseProperty::Pointer& prop, TiXmlElement* elem) const
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

void mitk::Label::SetProperty(const std::string &propertyKey, BaseProperty *property)
{

  itk::SimpleMemberCommand<Label>::Pointer command = itk::SimpleMemberCommand<Label>::New();
  command->SetCallbackFunction(this, &Label::Modified);
  property->AddObserver( itk::ModifiedEvent(), command );

  Superclass::SetProperty(propertyKey,property);
}

void mitk::Label::SetLocked(bool locked)
{
  mitk::BoolProperty* property = dynamic_cast<mitk::BoolProperty *>(GetProperty("locked"));
  if(property != NULL)
    // Update Property
    property->SetValue(locked);
  else
    // Create new Property
    SetBoolProperty("locked", locked);
}

bool mitk::Label::GetLocked() const
{
  bool locked;
  GetBoolProperty("locked",locked);
  return locked;
}

void mitk::Label::SetVisible(bool visible)
{
  mitk::BoolProperty* property = dynamic_cast<mitk::BoolProperty *>(GetProperty("visible"));
  if(property != NULL)
    // Update Property
    property->SetValue(visible);
  else
    // Create new Property
    SetBoolProperty("visible", visible);
}

bool mitk::Label::GetVisible() const
{
  bool visible;
  GetBoolProperty("visible",visible);
  return visible;
}

void mitk::Label::SetOpacity(float opacity)
{
  mitk::FloatProperty* property = dynamic_cast<mitk::FloatProperty *>(GetProperty("opacity"));
  if(property != NULL)
    // Update Property
    property->SetValue(opacity);
  else
    // Create new Property
    SetFloatProperty("opacity", opacity);
}

float mitk::Label::GetOpacity() const
{
  float opacity;
  GetFloatProperty("opacity",opacity);
  return opacity;
}

void mitk::Label::SetName(const std::string & name)
{
  SetStringProperty("name", name.c_str());
}

std::string mitk::Label::GetName() const
{
  std::string name;
  GetStringProperty("name",name);
  return name;
}

void mitk::Label::SetValue(int pixelValue)
{
  mitk::IntProperty* property = dynamic_cast<mitk::IntProperty *>(GetProperty("value"));
  if(property != NULL)
    // Update Property
    property->SetValue(pixelValue);
  else
    // Create new Property
    SetIntProperty("value", pixelValue);
}

int mitk::Label::GetValue() const
{
  int pixelValue;
  GetIntProperty("value",pixelValue);
  return pixelValue;
}

void mitk::Label::SetLayer(int layer)
{
  mitk::IntProperty* property = dynamic_cast<mitk::IntProperty *>(GetProperty("layer"));
  if(property != NULL)
    // Update Property
    property->SetValue(layer);
  else
    // Create new Property
    SetIntProperty("layer", layer);
}

int mitk::Label::GetLayer() const
{
  int layer;
  GetIntProperty("layer",layer);
  return layer;
}

const mitk::Color & mitk::Label::GetColor() const
{
  mitk::ColorProperty* colorProp = dynamic_cast<mitk::ColorProperty *>(GetProperty("color"));
  return colorProp->GetColor();
}

void mitk::Label::SetColor(const mitk::Color &_color)
{
  mitk::ColorProperty* colorProp = dynamic_cast<mitk::ColorProperty *>(GetProperty("color"));
  if(colorProp != NULL)
    // Update Property
    colorProp->SetColor(_color);
  else
    // Create new Property
    SetProperty("color", mitk::ColorProperty::New(_color));
}

void mitk::Label::SetCenterOfMassIndex(const mitk::Point3D& center)
{
  mitk::Point3dProperty* property = dynamic_cast<mitk::Point3dProperty *>(GetProperty("center.index"));
  if(property != NULL)
    // Update Property
    property->SetValue(center);
  else
    // Create new Property
    SetProperty("center.index", mitk::Point3dProperty::New(center));
}

mitk::Point3D mitk::Label::GetCenterOfMassIndex() const
{
  mitk::Point3dProperty* property = dynamic_cast<mitk::Point3dProperty *>(GetProperty("center.index"));
  return property->GetValue();
}

void mitk::Label::SetCenterOfMassCoordinates(const mitk::Point3D& center)
{
  mitk::Point3dProperty* property = dynamic_cast<mitk::Point3dProperty *>(GetProperty("center.coordinates"));
  if(property != NULL)
    // Update Property
    property->SetValue(center);
  else
    // Create new Property
    SetProperty("center.coordinates", mitk::Point3dProperty::New(center));
}

mitk::Point3D mitk::Label::GetCenterOfMassCoordinates() const
{
  mitk::Point3dProperty* property = dynamic_cast<mitk::Point3dProperty *>(GetProperty("center.coordinates"));
  return property->GetValue();
}

itk::LightObject::Pointer mitk::Label::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}

void mitk::Label::PrintSelf(std::ostream &os, itk::Indent indent) const
{
 // todo
}
