/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryIBerryPreferences.h"

namespace berry {

IBerryPreferences::~IBerryPreferences ()
{
}

IBerryPreferences::ChangeEvent::ChangeEvent(IBerryPreferences* source, const QString& property,
                                            const QString& oldValue, const QString& newValue)
  : m_Source(source)
  , m_Property(property)
  , m_OldValue(oldValue)
  , m_NewValue(newValue)
{
}

IBerryPreferences* IBerryPreferences::ChangeEvent::GetSource() const
{
  return m_Source;
}

QString IBerryPreferences::ChangeEvent::GetProperty() const
{
  return m_Property;
}

QString IBerryPreferences::ChangeEvent::GetOldValue() const
{
  return m_OldValue;
}

QString IBerryPreferences::ChangeEvent::GetNewValue() const
{
  return m_NewValue;
}

}
