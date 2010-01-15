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

#include "cherryCommandCategoryEvent.h"
#include "cherryCommandCategory.h"

namespace cherry
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
