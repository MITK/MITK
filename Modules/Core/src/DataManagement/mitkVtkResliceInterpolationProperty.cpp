/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkVtkResliceInterpolationProperty.h"
#include <vtkProperty.h>

mitk::VtkResliceInterpolationProperty::VtkResliceInterpolationProperty()
{
  this->AddInterpolationTypes();
  this->SetValue(static_cast<IdType>(VTK_RESLICE_NEAREST));
}

mitk::VtkResliceInterpolationProperty::VtkResliceInterpolationProperty(const IdType &value)
{
  this->AddInterpolationTypes();
  if (IsValidEnumerationValue(value))
  {
    this->SetValue(value);
  }
  else
  {
    this->SetValue(static_cast<IdType>(VTK_RESLICE_NEAREST));
  }
}

mitk::VtkResliceInterpolationProperty::VtkResliceInterpolationProperty(const std::string &value)
{
  this->AddInterpolationTypes();
  if (IsValidEnumerationValue(value))
  {
    this->SetValue(value);
  }
  else
  {
    this->SetValue(static_cast<IdType>(VTK_RESLICE_NEAREST));
  }
}

int mitk::VtkResliceInterpolationProperty::GetInterpolation()
{
  return static_cast<int>(this->GetValueAsId());
}

void mitk::VtkResliceInterpolationProperty::SetInterpolationToNearest()
{
  this->SetValue(static_cast<IdType>(VTK_RESLICE_NEAREST));
}

void mitk::VtkResliceInterpolationProperty::SetInterpolationToLinear()
{
  this->SetValue(static_cast<IdType>(VTK_RESLICE_LINEAR));
}

void mitk::VtkResliceInterpolationProperty::SetInterpolationToCubic()
{
  this->SetValue(static_cast<IdType>(VTK_RESLICE_CUBIC));
}

void mitk::VtkResliceInterpolationProperty::AddInterpolationTypes()
{
  AddEnum("Nearest", static_cast<IdType>(VTK_RESLICE_NEAREST));
  AddEnum("Linear", static_cast<IdType>(VTK_RESLICE_LINEAR));
  AddEnum("Cubic", static_cast<IdType>(VTK_RESLICE_CUBIC));
}

bool mitk::VtkResliceInterpolationProperty::AddEnum(const std::string &name, const IdType &id)
{
  return Superclass::AddEnum(name, id);
}

itk::LightObject::Pointer mitk::VtkResliceInterpolationProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}
