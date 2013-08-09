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


#include "mitkLabelSetProperty.h"

mitk::LabelSetProperty::LabelSetProperty()
{
}

mitk::LabelSetProperty::LabelSetProperty(const LabelSetProperty& other)
  : mitk::BaseProperty(other)
  , m_LookupTable(other.m_LookupTable)
{
}

mitk::LabelSetProperty::LabelSetProperty(const mitk::LookupTable::Pointer lut)
{
    this->SetLookupTable(lut);
}

bool mitk::LabelSetProperty::IsEqual(const BaseProperty& property) const
{
    return *(this->m_LookupTable) == *(static_cast<const Self&>(property).m_LookupTable);
}

bool mitk::LabelSetProperty::Assign(const BaseProperty& property)
{
  this->m_LookupTable = static_cast<const Self&>(property).m_LookupTable;
  return true;
}

std::string mitk::LabelSetProperty::GetValueAsString() const
{
  std::stringstream ss;
  ss << m_LookupTable;
  return ss.str();
}

mitk::LabelSetProperty::ValueType mitk::LabelSetProperty::GetValue() const
{
  return m_LookupTable;
}

void mitk::LabelSetProperty::SetLookupTable(const mitk::LookupTable::Pointer aLookupTable)
{
    if((m_LookupTable != aLookupTable) || (*m_LookupTable != *aLookupTable))
    {
        m_LookupTable = aLookupTable;
        Modified();
    }
}

void mitk::LabelSetProperty::SetValue(const ValueType & value)
{
  SetLookupTable(value);
}

itk::LightObject::Pointer mitk::LabelSetProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  return result;
}
