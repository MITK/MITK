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


#include "mitkLookupTableProperty.h"

mitk::LookupTableProperty::LookupTableProperty()
{
    mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
    this->SetLookupTable(lut);
}

mitk::LookupTableProperty::LookupTableProperty(const LookupTableProperty& other)
  : mitk::BaseProperty(other)
  , m_LookupTable(other.m_LookupTable)
{
}

mitk::LookupTableProperty::LookupTableProperty(const mitk::LookupTable::Pointer lut)
{
    this->SetLookupTable(lut);
}

bool mitk::LookupTableProperty::IsEqual(const BaseProperty& property) const
{
    return *(this->m_LookupTable) == *(static_cast<const Self&>(property).m_LookupTable);
}

bool mitk::LookupTableProperty::Assign(const BaseProperty& property)
{
  this->m_LookupTable = static_cast<const Self&>(property).m_LookupTable;
  return true;
}

std::string mitk::LookupTableProperty::GetValueAsString() const
{
  std::stringstream ss;
  ss << m_LookupTable;
  return ss.str();
}

mitk::LookupTableProperty::ValueType mitk::LookupTableProperty::GetValue() const
{
  return m_LookupTable;
}

void mitk::LookupTableProperty::SetLookupTable(const mitk::LookupTable::Pointer aLookupTable)
{

    if((m_LookupTable != aLookupTable) || (*m_LookupTable != *aLookupTable))
    {
        m_LookupTable = aLookupTable;
        Modified();
    }

}

void mitk::LookupTableProperty::SetValue(const ValueType & value)
{
  SetLookupTable(value);
}

itk::LightObject::Pointer mitk::LookupTableProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}
