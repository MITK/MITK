/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkGridRepresentationProperty.h"
#include <vtkProperty.h>

mitk::GridRepresentationProperty::GridRepresentationProperty()
{
  AddRepresentationTypes();
  SetValue(WIREFRAME);
}

mitk::GridRepresentationProperty::GridRepresentationProperty(const mitk::GridRepresentationProperty &other)
  : mitk::EnumerationProperty(other)
{
}

mitk::GridRepresentationProperty::GridRepresentationProperty(const IdType &value)
{
  AddRepresentationTypes();
  if (IsValidEnumerationValue(value))
  {
    SetValue(value);
  }
  else
  {
    SetValue(WIREFRAME);
  }
}

mitk::GridRepresentationProperty::GridRepresentationProperty(const std::string &value)
{
  AddRepresentationTypes();
  if (IsValidEnumerationValue(value))
  {
    SetValue(value);
  }
  else
  {
    SetValue(WIREFRAME);
  }
}

void mitk::GridRepresentationProperty::SetRepresentationToPoints()
{
  SetValue(POINTS);
}

void mitk::GridRepresentationProperty::SetRepresentationToWireframe()
{
  SetValue(WIREFRAME);
}

void mitk::GridRepresentationProperty::SetRepresentationToSurface()
{
  SetValue(SURFACE);
}

void mitk::GridRepresentationProperty::SetRepresentationToWireframeSurface()
{
  // SetValue( WIREFRAME_SURFACE );
}

void mitk::GridRepresentationProperty::AddRepresentationTypes()
{
  AddEnum("Points", POINTS);
  AddEnum("Wireframe", WIREFRAME);
  AddEnum("Surface", SURFACE);
  // AddEnum( "WireframeSurface", WIREFRAME_SURFACE );
}

bool mitk::GridRepresentationProperty::AddEnum(const std::string &name, const IdType &id)
{
  return Superclass::AddEnum(name, id);
}

itk::LightObject::Pointer mitk::GridRepresentationProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}
