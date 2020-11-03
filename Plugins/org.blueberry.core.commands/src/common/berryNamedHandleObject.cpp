/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryNamedHandleObject.h"

#include "berryCommandExceptions.h"

namespace berry
{

NamedHandleObject::NamedHandleObject(const QString& id) :
  HandleObject(id)
{

}

QString NamedHandleObject::GetDescription() const
{
  if (!this->IsDefined())
  {
    throw NotDefinedException(
        "Cannot get a description from an undefined object. " //$NON-NLS-1$
        + id);
  }

  return description;
}

QString NamedHandleObject::GetName() const
{
  if (!this->IsDefined())
  {
    throw NotDefinedException(
        "Cannot get the name from an undefined object. " + id);
  }

  return name;
}

}
