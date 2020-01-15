/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryCommandCategoryEvent.h"
#include "berryCommandCategory.h"

namespace berry
{

CommandCategoryEvent::CommandCategoryEvent(
    const SmartPointer<CommandCategory> category, bool definedChanged,
    bool descriptionChanged, bool nameChanged) :
  AbstractNamedHandleEvent(definedChanged, descriptionChanged, nameChanged),
      category(category)
{

  if (!category)
  {
    throw Poco::NullPointerException("CommandCategory cannot be null");
  }
}

SmartPointer<CommandCategory> CommandCategoryEvent::GetCategory() const
{
  return category;
}

}
