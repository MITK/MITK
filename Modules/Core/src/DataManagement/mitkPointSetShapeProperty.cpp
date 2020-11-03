/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPointSetShapeProperty.h"

mitk::PointSetShapeProperty::PointSetShapeProperty()
{
  this->AddPointSetShapes();
  this->SetValue(CROSS);
}

mitk::PointSetShapeProperty::PointSetShapeProperty(const IdType &value)
{
  this->AddPointSetShapes();
  if (IsValidEnumerationValue(value))
  {
    this->SetValue(value);
  }
  else
    MITK_WARN << "Warning: invalid point set shape";
}

mitk::PointSetShapeProperty::PointSetShapeProperty(const std::string &value)
{
  this->AddPointSetShapes();
  if (IsValidEnumerationValue(value))
  {
    this->SetValue(value);
  }
  else
    MITK_WARN << "Invalid point set shape";
}

int mitk::PointSetShapeProperty::GetPointSetShape() const
{
  return static_cast<int>(this->GetValueAsId());
}

void mitk::PointSetShapeProperty::AddPointSetShapes()
{
  AddEnum("None", NONE);
  AddEnum("Vertex", VERTEX);
  AddEnum("Dash", DASH);
  AddEnum("Cross", CROSS);
  AddEnum("ThickCross", THICK_CROSS);
  AddEnum("Triangle", TRIANGLE);
  AddEnum("Square", SQUARE);
  AddEnum("Circle", CIRCLE);
  AddEnum("Diamond", DIAMOND);
  AddEnum("Arrow", ARROW);
  AddEnum("ThickArrow", THICK_ARROW);
  AddEnum("HookedArrow", HOOKED_ARROW);
}

bool mitk::PointSetShapeProperty::AddEnum(const std::string &name, const IdType &id)
{
  return Superclass::AddEnum(name, id);
}

itk::LightObject::Pointer mitk::PointSetShapeProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}
