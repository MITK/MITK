/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
