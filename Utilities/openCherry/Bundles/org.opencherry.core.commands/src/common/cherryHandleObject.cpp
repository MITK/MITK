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
  if (typeid(object) != typeid(this))
  {
    return false;
  }

  // Check each property in turn.
  const HandleObject* handle = dynamic_cast<const HandleObject*>(object);
  return (id == handle->id);
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
