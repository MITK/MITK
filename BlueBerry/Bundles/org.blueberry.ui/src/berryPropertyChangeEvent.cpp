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

#include "berryPropertyChangeEvent.h"

namespace berry
{

PropertyChangeEvent::PropertyChangeEvent(Object::Pointer source,
    const std::string& property, Object::Pointer oldValue,
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

std::string PropertyChangeEvent::GetProperty()
{
  return propertyName;
}

Object::Pointer PropertyChangeEvent::GetSource()
{
  return source;
}

}
