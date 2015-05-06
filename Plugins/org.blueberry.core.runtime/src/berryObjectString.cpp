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
