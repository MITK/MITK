/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

bool KeyedElement::operator ==(const Object* other) const
{
  if (other == nullptr) return false;
  return (typeid(this) == typeid(other) && IsEqual(*static_cast<const KeyedElement*>(other)));
}

uint KeyedElement::HashCode() const
{
  return qHash(this->GetKey());
}

}
