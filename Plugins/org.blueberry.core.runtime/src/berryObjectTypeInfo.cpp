/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

