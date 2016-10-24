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

#include "mitkWeakPointerProperty.h"

bool mitk::WeakPointerProperty::IsEqual(const BaseProperty &property) const
{
  return this->m_WeakPointer == static_cast<const Self &>(property).m_WeakPointer;
}

bool mitk::WeakPointerProperty::Assign(const BaseProperty &property)
{
  this->m_WeakPointer = static_cast<const Self &>(property).m_WeakPointer;
  return true;
}

mitk::WeakPointerProperty::WeakPointerProperty(itk::Object *pointer) : m_WeakPointer(pointer)
{
}

mitk::WeakPointerProperty::WeakPointerProperty(const WeakPointerProperty &other)
  : mitk::BaseProperty(other), m_WeakPointer(other.m_WeakPointer)
{
}

mitk::WeakPointerProperty::~WeakPointerProperty()
{
}

std::string mitk::WeakPointerProperty::GetValueAsString() const
{
  std::stringstream ss;
  ss << m_WeakPointer.GetPointer();
  return ss.str();
}

mitk::WeakPointerProperty::ValueType mitk::WeakPointerProperty::GetWeakPointer() const
{
  return m_WeakPointer.GetPointer();
}

mitk::WeakPointerProperty::ValueType mitk::WeakPointerProperty::GetValue() const
{
  return GetWeakPointer();
}

void mitk::WeakPointerProperty::SetWeakPointer(itk::Object *pointer)
{
  if (m_WeakPointer.GetPointer() != pointer)
  {
    m_WeakPointer = pointer;
    Modified();
  }
}

void mitk::WeakPointerProperty::SetValue(const ValueType &value)
{
  SetWeakPointer(value.GetPointer());
}

itk::LightObject::Pointer mitk::WeakPointerProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}
