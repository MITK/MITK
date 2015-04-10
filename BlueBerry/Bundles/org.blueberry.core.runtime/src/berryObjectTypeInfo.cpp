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

#include "berryObjectTypeInfo.h"

namespace berry {

ObjectTypeInfo::ObjectTypeInfo()
{

}

ObjectTypeInfo::ObjectTypeInfo(const Reflection::TypeInfo &typeInfo)
  : Reflection::TypeInfo(typeInfo)
{
}

bool ObjectTypeInfo::operator==(const Object *other) const
{
  if (const ObjectTypeInfo* otherStr = dynamic_cast<const ObjectTypeInfo*>(other))
  {
    return static_cast<const Reflection::TypeInfo&>(*this) == static_cast<const Reflection::TypeInfo&>(*otherStr);
  }
  return false;
}

bool ObjectTypeInfo::operator==(const Reflection::TypeInfo &other) const
{
  return static_cast<const Reflection::TypeInfo&>(*this) == other;
}

}

