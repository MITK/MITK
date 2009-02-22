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

#include "cherryHandleObject.h"
#include <typeinfo>

namespace cherry
{

HandleObject::HandleObject(const std::string& ID) :
  defined(false), id(ID)
{
}

bool HandleObject::operator==(const Object* object) const
{
  // Check if they're the same.
  if (object == this)
  {
    return true;
  }

  // Check if they're the same type.
  if (const Self* o = dynamic_cast<const Self*>(object))
  {
    // Check each property in turn.
    return (id == o->id);
  }

  return false;
}

std::string HandleObject::GetId() const
{
  return id;
}

bool HandleObject::IsDefined() const
{
  return defined;
}

}
