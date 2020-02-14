/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkModalityProperty.h"

mitk::ModalityProperty::ModalityProperty()
{
  AddEnumerationTypes();
}

mitk::ModalityProperty::ModalityProperty(const IdType &value)
{
  AddEnumerationTypes();
  if (IsValidEnumerationValue(value))
  {
    SetValue(value);
  }
  else
  {
    SetValue(0);
  }
}

mitk::ModalityProperty::ModalityProperty(const std::string &value)
{
  AddEnumerationTypes();
  if (IsValidEnumerationValue(value))
  {
    SetValue(value);
  }
  else
  {
    SetValue("undefined");
  }
}

mitk::ModalityProperty::~ModalityProperty()
{
}

void mitk::ModalityProperty::AddEnumerationTypes()
{
  auto newId = static_cast<IdType>(EnumerationProperty::Size());

  AddEnum("undefined", newId++);
  AddEnum("CR", newId++);            // computer radiography
  AddEnum("CT", newId++);            // computed tomography
  AddEnum("MR", newId++);            // magnetic resonance
  AddEnum("NM", newId++);            // nuclear medicine
  AddEnum("US", newId++);            // ultrasound
  AddEnum("Color Doppler", newId++); // ultrasound
  AddEnum("Power Doppler", newId++); // ultrasound
}

itk::LightObject::Pointer mitk::ModalityProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}
