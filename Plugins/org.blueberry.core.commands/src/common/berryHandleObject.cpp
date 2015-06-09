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

#include "berryHandleObject.h"
#include <typeinfo>

namespace berry
{

const uint HandleObject::HASH_CODE_NOT_COMPUTED = 0;
const uint HandleObject::HASH_FACTOR = 89;
const uint HandleObject::HASH_INITIAL = qHash(HandleObject::GetStaticClassName());

HandleObject::HandleObject(const QString& ID) :
  hashCode(HASH_CODE_NOT_COMPUTED), defined(false), id(ID)
{
}

bool HandleObject::operator==(const Object* object) const
{
  // Check if they're the same.
  if (object == this)
  {
    return true;
  }

  // Check if they're the same type.
  if (const Self* o = dynamic_cast<const Self*>(object))
  {
    // Check each property in turn.
    return (id == o->id);
  }

  return false;
}

QString HandleObject::GetId() const
{
  return id;
}

bool HandleObject::IsDefined() const
{
  return defined;
}

}
