/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryPropertyChangeEvent.h"

namespace berry
{

PropertyChangeEvent::PropertyChangeEvent(Object::Pointer source,
    const QString& property, Object::Pointer oldValue,
    Object::Pointer newValue)
{
  this->source = source;
  this->propertyName = property;
  this->oldValue = oldValue;
  this->newValue = newValue;
}

Object::Pointer PropertyChangeEvent::GetNewValue()
{
  return newValue;
}

Object::Pointer PropertyChangeEvent::GetOldValue()
{
  return oldValue;
}

QString PropertyChangeEvent::GetProperty()
{
  return propertyName;
}

Object::Pointer PropertyChangeEvent::GetSource()
{
  return source;
}

}
