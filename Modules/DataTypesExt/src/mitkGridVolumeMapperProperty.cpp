/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkGridVolumeMapperProperty.h"
#include <vtkProperty.h>

mitk::GridVolumeMapperProperty::GridVolumeMapperProperty()
{
  AddRepresentationTypes();
  SetValue(PT);
}

mitk::GridVolumeMapperProperty::GridVolumeMapperProperty(const IdType &value)
{
  AddRepresentationTypes();
  if (IsValidEnumerationValue(value))
  {
    SetValue(value);
  }
  else
  {
    SetValue(PT);
  }
}

mitk::GridVolumeMapperProperty::GridVolumeMapperProperty(const std::string &value)
{
  AddRepresentationTypes();
  if (IsValidEnumerationValue(value))
  {
    SetValue(value);
  }
  else
  {
    SetValue(PT);
  }
}

void mitk::GridVolumeMapperProperty::SetVolumeMapperToPT()
{
  SetValue(PT);
}

void mitk::GridVolumeMapperProperty::SetVolumeMapperToZSweep()
{
  SetValue(ZSWEEP);
}

void mitk::GridVolumeMapperProperty::SetVolumeMapperToRayCast()
{
  SetValue(RAYCAST);
}

void mitk::GridVolumeMapperProperty::AddRepresentationTypes()
{
  AddEnum("Ray Cast", RAYCAST);
  AddEnum("Projected Tetrahedra", PT);
  AddEnum("ZSweep", ZSWEEP);
}

bool mitk::GridVolumeMapperProperty::AddEnum(const std::string &name, const IdType &id)
{
  return Superclass::AddEnum(name, id);
}

itk::LightObject::Pointer mitk::GridVolumeMapperProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}
