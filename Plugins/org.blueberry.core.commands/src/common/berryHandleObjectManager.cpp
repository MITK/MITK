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

#include "berryHandleObjectManager.h"

#include "berryHandleObject.h"

namespace berry
{

void HandleObjectManager::CheckId(const QString& id) const
{
  if (id.isEmpty())
  {
    throw ctkInvalidArgumentException(
          "The handle object must not have a zero-length identifier");
  }
}

QSet<QString> HandleObjectManager::GetDefinedHandleObjectIds() const
{
  QSet<QString> definedHandleObjectIds;
  definedHandleObjectIds.reserve(definedHandleObjects.size());
  foreach (HandleObject::Pointer ho, definedHandleObjects)
  {
    definedHandleObjectIds.insert(ho->GetId());
  }
  return definedHandleObjectIds;
}

}
