/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "cherryHandleObjectManager.h"

namespace cherry
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
