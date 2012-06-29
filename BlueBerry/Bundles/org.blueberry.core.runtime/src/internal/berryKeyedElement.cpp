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

#include "berryKeyedElement.h"

#include <typeinfo>

namespace berry {

KeyedElement::~KeyedElement()
{
}

bool KeyedElement::operator ==(const KeyedElement& other) const
{
  return (typeid(*this) == typeid(other) && IsEqual(other));
}

bool KeyedElement::operator ==(const KeyedElement* other) const
{
  if (other == NULL) return false;
  return (typeid(this) == typeid(other) && IsEqual(*other));
}

uint qHash(const berry::SmartPointer<berry::KeyedElement>& element)
{
  return qHash(element->GetKey());
}

}
