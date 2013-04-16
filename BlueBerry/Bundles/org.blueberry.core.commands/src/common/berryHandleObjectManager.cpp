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

namespace berry
{

void HandleObjectManager::CheckId(const std::string& id) const
{
  if (id.empty())
  {
    throw std::invalid_argument(
        "The handle object must not have a zero-length identifier"); //$NON-NLS-1$
  }
}

Poco::HashSet<std::string> HandleObjectManager::GetDefinedHandleObjectIds() const
{
  Poco::HashSet<std::string> definedHandleObjectIds(definedHandleObjects.size());
  for (Poco::HashSet<HandleObject::Pointer, HandleObject::Hash>::ConstIterator iter =
      definedHandleObjects.begin(); iter != definedHandleObjects.end(); ++iter)
  {
    const std::string id((*iter)->GetId());
    definedHandleObjectIds.insert(id);
  }
  return definedHandleObjectIds;
}

}
