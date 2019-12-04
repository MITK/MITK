/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkResliceMethodProperty.h"

mitk::ResliceMethodProperty::ResliceMethodProperty()
{
  AddThickSlicesTypes();
  SetValue((IdType)0);
}

mitk::ResliceMethodProperty::ResliceMethodProperty(const IdType &value)
{
  AddThickSlicesTypes();
  if (IsValidEnumerationValue(value))
    SetValue(value);
}

mitk::ResliceMethodProperty::ResliceMethodProperty(const std::string &value)
{
  AddThickSlicesTypes();
  if (IsValidEnumerationValue(value))
    SetValue(value);
}

void mitk::ResliceMethodProperty::AddThickSlicesTypes()
{
  AddEnum("disabled", (IdType)0);
  AddEnum("mip", (IdType)1);
  AddEnum("sum", (IdType)2);
  AddEnum("weighted", (IdType)3);
  AddEnum("minip", (IdType)4);
  AddEnum("mean", (IdType)5);
}

itk::LightObject::Pointer mitk::ResliceMethodProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}
