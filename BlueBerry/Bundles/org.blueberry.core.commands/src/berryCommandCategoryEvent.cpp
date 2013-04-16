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
