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

#include "berryIObjectManager.h"

namespace berry {

Handle::Handle(const SmartPointer<const IObjectManager> &objectManager, int value)
  : objectManager(objectManager.GetPointer())
  , objectId(value)
  , isStrongRef(true)
{
  const_cast<IObjectManager*>(this->objectManager)->Register();
}

Handle::Handle(const IObjectManager *objectManager, int value)
  : objectManager(objectManager)
  , objectId(value)
  , isStrongRef(false)
{
}

Handle::~Handle()
{
  if (isStrongRef)
  {
    const_cast<IObjectManager*>(objectManager)->UnRegister();
  }
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
