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

#include <mitkStringProperty.h>


mitk::Label::Label():
  PropertyList()
{
  // initialize basic label properties
//  SetLocked(true);
//  SetVisible(true);
//  SetOpacity(0.6);
//  SetValue(-1);
//  SetLayer(0);
//  mitk::Point3D pnt;
//  pnt.SetElement(0,0);
//  pnt.SetElement(1,0);
//  pnt.SetElement(2,0);
//  SetCenterOfMassCoordinates(pnt);
//  SetCenterOfMassIndex(pnt);

//  SetName("NoName");

//  mitk::Color col;
//  col.Set(0,0,0);
//  SetColor(col);
}

mitk::Label::Label(const Label& other)
  : PropertyList(other)
  // copyconstructer of property List handles the coping action
{
}

mitk::Label::~Label()
{

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

bool mitk::Label::GetLocked()
{
  if(GetProperty("locked") == NULL) SetLocked(true);

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

bool mitk::Label::GetVisible()
{
  if(GetProperty("visible") == NULL) SetVisible(true);

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

float mitk::Label::GetOpacity()
{
  if(GetProperty("opacity") == NULL) SetOpacity(0.6);

  float opacity;
  GetFloatProperty("opacity",opacity);
  return opacity;
}

void mitk::Label::SetName(const std::string & name)
{
  SetStringProperty("name", name.c_str());
}

std::string mitk::Label::GetName()
{
  if(GetProperty("name") == NULL) SetName("noName!");

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

int mitk::Label::GetValue()
{
  if(GetProperty("value") == NULL) SetValue(-1);

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

int mitk::Label::GetLayer()
{
  if(GetProperty("layer") == NULL) SetLayer(-1);

  int layer;
  GetIntProperty("layer",layer);
  return layer;
}

const mitk::Color & mitk::Label::GetColor()
{
  if(GetProperty("color") == NULL)
  {
    mitk::Color col;
    col.Set(0,0,0);
    SetColor(col);
  }

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

mitk::Point3D mitk::Label::GetCenterOfMassIndex()
{
  if(GetProperty("center.index") == NULL)
  {
    mitk::Point3D pnt;
    pnt.SetElement(0,0);
    pnt.SetElement(1,0);
    pnt.SetElement(2,0);
    SetCenterOfMassIndex(pnt);
  }

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

mitk::Point3D mitk::Label::GetCenterOfMassCoordinates()
{
  if(GetProperty("center.coordinates") == NULL)
  {
    mitk::Point3D pnt;
    pnt.SetElement(0,0);
    pnt.SetElement(1,0);
    pnt.SetElement(2,0);
    SetCenterOfMassCoordinates(pnt);
  }

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
