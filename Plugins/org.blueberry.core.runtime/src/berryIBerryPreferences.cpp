/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
