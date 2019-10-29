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
#include "tinyxml.h"
#include <itkCommand.h>
#include <mitkProperties.h>
#include <mitkDICOMSegmentationPropertyHelper.h>
#include <mitkStringProperty.h>

const mitk::Label::PixelType mitk::Label::MAX_LABEL_VALUE = std::numeric_limits<mitk::Label::PixelType>::max();

mitk::Label::Label() : PropertyList()
{
  if (GetProperty("locked") == nullptr)
    SetLocked(true);
  if (GetProperty("visible") == nullptr)
    SetVisible(true);
  if (GetProperty("opacity") == nullptr)
    SetOpacity(0.6);
  if (GetProperty("center.coordinates") == nullptr)
  {
    mitk::Point3D pnt;
    pnt.SetElement(0, 0);
    pnt.SetElement(1, 0);
    pnt.SetElement(2, 0);
    SetCenterOfMassCoordinates(pnt);
  }
  if (GetProperty("center.index") == nullptr)
  {
    mitk::Point3D pnt;
    pnt.SetElement(0, 0);
    pnt.SetElement(1, 0);
    pnt.SetElement(2, 0);
    SetCenterOfMassIndex(pnt);
  }
  if (GetProperty("color") == nullptr)
  {
    mitk::Color col;
    col.Set(0, 0, 0);
    SetColor(col);
  }
  if (GetProperty("name") == nullptr)
    SetName("noName!");
  if (GetProperty("value") == nullptr)
    SetValue(0);
  if (GetProperty("layer") == nullptr)
    SetLayer(0);

  DICOMSegmentationPropertyHelper::SetDICOMSegmentProperties(this);
}

mitk::Label::Label(const Label &other) : PropertyList(other)
// copyconstructer of property List handles the coping action
{
  auto *map = this->GetMap();
  auto it = map->begin();
  auto end = map->end();

  for (; it != end; ++it)
  {
    itk::SimpleMemberCommand<Label>::Pointer command = itk::SimpleMemberCommand<Label>::New();
    command->SetCallbackFunction(this, &Label::Modified);
    it->second->AddObserver(itk::ModifiedEvent(), command);
  }
}

mitk::Label::~Label()
{
}

void mitk::Label::SetProperty(const std::string &propertyKey, BaseProperty *property, const std::string &contextName, bool fallBackOnDefaultContext)
{
  itk::SimpleMemberCommand<Label>::Pointer command = itk::SimpleMemberCommand<Label>::New();
  command->SetCallbackFunction(this, &Label::Modified);
  property->AddObserver(itk::ModifiedEvent(), command);

  Superclass::SetProperty(propertyKey, property, contextName, fallBackOnDefaultContext);
}

void mitk::Label::SetLocked(bool locked)
{
  mitk::BoolProperty *property = dynamic_cast<mitk::BoolProperty *>(GetProperty("locked"));
  if (property != nullptr)
    // Update Property
    property->SetValue(locked);
  else
    // Create new Property
    SetBoolProperty("locked", locked);
}

bool mitk::Label::GetLocked() const
{
  bool locked;
  GetBoolProperty("locked", locked);
  return locked;
}

void mitk::Label::SetVisible(bool visible)
{
  mitk::BoolProperty *property = dynamic_cast<mitk::BoolProperty *>(GetProperty("visible"));
  if (property != nullptr)
    // Update Property
    property->SetValue(visible);
  else
    // Create new Property
    SetBoolProperty("visible", visible);
}

bool mitk::Label::GetVisible() const
{
  bool visible;
  GetBoolProperty("visible", visible);
  return visible;
}

void mitk::Label::SetOpacity(float opacity)
{
  mitk::FloatProperty *property = dynamic_cast<mitk::FloatProperty *>(GetProperty("opacity"));
  if (property != nullptr)
    // Update Property
    property->SetValue(opacity);
  else
    // Create new Property
    SetFloatProperty("opacity", opacity);
}

float mitk::Label::GetOpacity() const
{
  float opacity;
  GetFloatProperty("opacity", opacity);
  return opacity;
}

void mitk::Label::SetName(const std::string &name)
{
  SetStringProperty("name", name.c_str());
}

std::string mitk::Label::GetName() const
{
  std::string name;
  GetStringProperty("name", name);
  return name;
}

void mitk::Label::SetValue(PixelType pixelValue)
{
  mitk::UShortProperty *property = dynamic_cast<mitk::UShortProperty *>(GetProperty("value"));
  if (property != nullptr)
    // Update Property
    property->SetValue(pixelValue);
  else
    // Create new Property
    SetProperty("value", mitk::UShortProperty::New(pixelValue));
}

mitk::Label::PixelType mitk::Label::GetValue() const
{
  PixelType pixelValue;
  mitk::UShortProperty *property = dynamic_cast<UShortProperty *>(GetProperty("value"));
  assert(property);
  pixelValue = property->GetValue();
  return pixelValue;
}

void mitk::Label::SetLayer(unsigned int layer)
{
  mitk::UIntProperty *property = dynamic_cast<mitk::UIntProperty *>(GetProperty("layer"));
  if (property != nullptr)
    // Update Property
    property->SetValue(layer);
  else
    // Create new Property
    SetProperty("layer", mitk::UIntProperty::New(layer));
}

unsigned int mitk::Label::GetLayer() const
{
  unsigned int layer;
  mitk::UIntProperty *prop = dynamic_cast<mitk::UIntProperty *>(GetProperty("layer"));
  layer = prop->GetValue();
  return layer;
}

const mitk::Color &mitk::Label::GetColor() const
{
  mitk::ColorProperty *colorProp = dynamic_cast<mitk::ColorProperty *>(GetProperty("color"));
  return colorProp->GetColor();
}

void mitk::Label::SetColor(const mitk::Color &_color)
{
  mitk::ColorProperty *colorProp = dynamic_cast<mitk::ColorProperty *>(GetProperty("color"));
  if (colorProp != nullptr)
    // Update Property
    colorProp->SetColor(_color);
  else
    // Create new Property
    SetProperty("color", mitk::ColorProperty::New(_color));
}

void mitk::Label::SetCenterOfMassIndex(const mitk::Point3D &center)
{
  mitk::Point3dProperty *property = dynamic_cast<mitk::Point3dProperty *>(GetProperty("center.index"));
  if (property != nullptr)
    // Update Property
    property->SetValue(center);
  else
    // Create new Property
    SetProperty("center.index", mitk::Point3dProperty::New(center));
}

mitk::Point3D mitk::Label::GetCenterOfMassIndex() const
{
  mitk::Point3dProperty *property = dynamic_cast<mitk::Point3dProperty *>(GetProperty("center.index"));
  return property->GetValue();
}

void mitk::Label::SetCenterOfMassCoordinates(const mitk::Point3D &center)
{
  mitk::Point3dProperty *property = dynamic_cast<mitk::Point3dProperty *>(GetProperty("center.coordinates"));
  if (property != nullptr)
    // Update Property
    property->SetValue(center);
  else
    // Create new Property
    SetProperty("center.coordinates", mitk::Point3dProperty::New(center));
}

mitk::Point3D mitk::Label::GetCenterOfMassCoordinates() const
{
  mitk::Point3dProperty *property = dynamic_cast<mitk::Point3dProperty *>(GetProperty("center.coordinates"));
  return property->GetValue();
}

itk::LightObject::Pointer mitk::Label::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}

void mitk::Label::PrintSelf(std::ostream & /*os*/, itk::Indent /*indent*/) const
{
  // todo
}

bool mitk::Equal(const mitk::Label &leftHandSide, const mitk::Label &rightHandSide, ScalarType /*eps*/, bool verbose)
{
  MITK_INFO(verbose) << "--- Label Equal ---";

  bool returnValue = true;
  // have to be replaced until a PropertyList Equal was implemented :
  // returnValue = mitk::Equal((const mitk::PropertyList &)leftHandSide,(const mitk::PropertyList
  // &)rightHandSide,eps,verbose);

  const mitk::PropertyList::PropertyMap *lhsmap = leftHandSide.GetMap();
  const mitk::PropertyList::PropertyMap *rhsmap = rightHandSide.GetMap();

  returnValue = lhsmap->size() == rhsmap->size();

  if (!returnValue)
  {
    MITK_INFO(verbose) << "Labels in label container are not equal.";
    return returnValue;
  }

  auto lhsmapIt = lhsmap->begin();
  auto lhsmapItEnd = lhsmap->end();

  for (; lhsmapIt != lhsmapItEnd; ++lhsmapIt)
  {
    if (rhsmap->find(lhsmapIt->first) == rhsmap->end())
    {
      returnValue = false;
      break;
    }
  }

  if (!returnValue)
  {
    MITK_INFO(verbose) << "Labels in label container are not equal.";
    return returnValue;
  }

  return returnValue;
}
