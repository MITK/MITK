/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIPreferences.h>

mitk::IPreferences::ChangeEvent::ChangeEvent(IPreferences* source, const std::string& property, const std::string& oldValue, const std::string& newValue)
  : m_Source(source),
    m_Property(property),
    m_OldValue(oldValue),
    m_NewValue(newValue)
{
}

mitk::IPreferences* mitk::IPreferences::ChangeEvent::GetSource() const
{
  return m_Source;
}

std::string mitk::IPreferences::ChangeEvent::GetProperty() const
{
  return m_Property;
}

std::string mitk::IPreferences::ChangeEvent::GetOldValue() const
{
  return m_OldValue;
}

std::string mitk::IPreferences::ChangeEvent::GetNewValue() const
{
  return m_NewValue;
}

mitk::IPreferences::~IPreferences()
{
}
