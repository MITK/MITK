/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
