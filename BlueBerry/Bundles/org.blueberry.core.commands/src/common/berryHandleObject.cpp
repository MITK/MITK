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

const std::size_t HandleObject::HASH_CODE_NOT_COMPUTED = 0;
const std::size_t HandleObject::HASH_FACTOR = 89;
const std::size_t HandleObject::HASH_INITIAL = Poco::hash("berry::HandleObject");

HandleObject::HandleObject(const std::string& ID) :
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

std::string HandleObject::GetId() const
{
  return id;
}

bool HandleObject::IsDefined() const
{
  return defined;
}

}
