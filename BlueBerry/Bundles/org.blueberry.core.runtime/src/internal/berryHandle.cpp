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


#include "berryHandle.h"

namespace berry {

Handle::Handle(RegistryObjectManager* objectManager, int value)
  : objectManager(objectManager), objectId(value)
{
}

int Handle::GetId() const
{
  return objectId;
}

bool Handle::operator==(const Object* object) const
{
  if (const Handle* h = dynamic_cast<const Handle*>(object))
  {
    return objectId == h->objectId;
  }
  return false;
}

uint Handle::HashCode() const
{
  return static_cast<std::size_t>(objectId);
}


}
