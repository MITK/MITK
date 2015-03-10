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


#include "mitkStringProperty.h"

const char* mitk::StringProperty::PATH = "path";
mitk::StringProperty::StringProperty( const char* string )
: m_Value()
{
  if ( string )
    m_Value = string;
}

mitk::StringProperty::StringProperty( const std::string& s )
: m_Value( s )
{
}

mitk::StringProperty::StringProperty(const StringProperty& other)
  : BaseProperty(other)
  , m_Value(other.m_Value)
{
}

bool mitk::StringProperty::IsEqual(const BaseProperty& property ) const
{
  return this->m_Value == static_cast<const Self&>(property).m_Value;
}

bool mitk::StringProperty::Assign(const BaseProperty& property )
{
  this->m_Value = static_cast<const Self&>(property).m_Value;
  return true;
}

std::string mitk::StringProperty::GetValueAsString() const
{
  return m_Value;
}

itk::LightObject::Pointer mitk::StringProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}
