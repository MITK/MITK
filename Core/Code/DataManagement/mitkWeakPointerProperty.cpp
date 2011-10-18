/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkWeakPointerProperty.h"

bool mitk::WeakPointerProperty::IsEqual(const BaseProperty& property) const
{
    return this->m_WeakPointer == static_cast<const Self&>(property).m_WeakPointer;
}

bool mitk::WeakPointerProperty::Assign(const BaseProperty& property)
{
    this->m_WeakPointer = static_cast<const Self&>(property).m_WeakPointer;
    return true;
}

mitk::WeakPointerProperty::WeakPointerProperty(itk::Object* pointer) : m_WeakPointer(pointer)
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

void mitk::WeakPointerProperty::SetWeakPointer(itk::Object* pointer)
{
    if(m_WeakPointer.GetPointer() != pointer)
    {
        m_WeakPointer = pointer;
        Modified();
    }
}

void mitk::WeakPointerProperty::SetValue(const ValueType &value)
{
  SetWeakPointer(value.GetPointer());
}
