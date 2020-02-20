/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
