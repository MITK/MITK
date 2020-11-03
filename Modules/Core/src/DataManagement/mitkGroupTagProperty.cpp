/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkGroupTagProperty.h"

mitk::GroupTagProperty::GroupTagProperty() : mitk::BaseProperty()
{
}

mitk::GroupTagProperty::GroupTagProperty(const GroupTagProperty &other) : mitk::BaseProperty(other)
{
}

bool mitk::GroupTagProperty::IsEqual(const BaseProperty & /*property*/) const
{
  // if other property is also a GroupTagProperty, then it is equal to us, because tags have no value themselves
  return true;
}

bool mitk::GroupTagProperty::Assign(const BaseProperty & /*property*/)
{
  return true;
}

itk::LightObject::Pointer mitk::GroupTagProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}
