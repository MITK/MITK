/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkBaseProperty.h"

const std::string mitk::BaseProperty::VALUE_CANNOT_BE_CONVERTED_TO_STRING = "n/a";

mitk::BaseProperty::BaseProperty()
{
}

mitk::BaseProperty::BaseProperty(const mitk::BaseProperty &) : itk::Object()
{
}

mitk::BaseProperty::~BaseProperty()
{
}

std::string mitk::BaseProperty::GetValueAsString() const
{
  return std::string(VALUE_CANNOT_BE_CONVERTED_TO_STRING);
}

mitk::BaseProperty &mitk::BaseProperty::operator=(const BaseProperty &rhs)
{
  AssignProperty(rhs);
  return *this;
}

bool mitk::BaseProperty::AssignProperty(const BaseProperty &rhs)
{
  if (this == &rhs)
    return true; // no self assignment

  if (typeid(*this) == typeid(rhs) && Assign(rhs))
  {
    this->Modified();
    return true;
  }
  return false;
}

bool mitk::BaseProperty::operator==(const BaseProperty &property) const
{
  return (typeid(*this) == typeid(property) && IsEqual(property));
}
