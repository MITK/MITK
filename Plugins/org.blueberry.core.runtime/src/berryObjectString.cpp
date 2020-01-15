/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryObjectString.h"

namespace berry {

ObjectString::ObjectString()
{}

ObjectString::ObjectString(const QString& s)
  : QString(s)
{}

bool ObjectString::operator==(const Object* other) const
{
  if (const ObjectString* otherStr = dynamic_cast<const ObjectString*>(other))
  {
    return static_cast<const QString&>(*this) == static_cast<const QString&>(*otherStr);
  }
  return false;
}

bool ObjectString::operator==(const QString& other) const
{
  return static_cast<const QString&>(*this) == other;
}

QString ObjectString::ToString() const
{
  return *this;
}

ObjectString::~ObjectString()
{
}

berry::ObjectString &berry::ObjectString::operator =(const QString &other)
{
  QString::operator =(other);
  return *this;
}

}
