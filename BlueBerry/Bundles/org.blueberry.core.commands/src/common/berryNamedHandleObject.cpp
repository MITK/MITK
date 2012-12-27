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

#include "berryNamedHandleObject.h"

#include "berryCommandExceptions.h"

namespace berry
{

NamedHandleObject::NamedHandleObject(const std::string& id) :
  HandleObject(id)
{

}

std::string NamedHandleObject::GetDescription() const
{
  if (!this->IsDefined())
  {
    throw NotDefinedException(
        "Cannot get a description from an undefined object. " //$NON-NLS-1$
        + id);
  }

  return description;
}

std::string NamedHandleObject::GetName() const
{
  if (!this->IsDefined())
  {
    throw NotDefinedException(
        "Cannot get the name from an undefined object. " //$NON-NLS-1$
        + id);
  }

  return name;
}

}
