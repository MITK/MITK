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

#include "cherryNamedHandleObject.h"

#include "cherryCommandExceptions.h"

namespace cherry
{

NamedHandleObject::NamedHandleObject(const std::string& id) :
  HandleObject(id)
{

}

std::string NamedHandleObject::GetDescription()
{
  if (!this->IsDefined())
  {
    throw NotDefinedException(
        "Cannot get a description from an undefined object. " //$NON-NLS-1$
        + id);
  }

  return description;
}

std::string NamedHandleObject::GetName()
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
