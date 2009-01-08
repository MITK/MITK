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

#include "cherryState.h"

namespace cherry
{

std::string State::GetId()
{
  return id;
}

Object::Pointer State::GetValue()
{
  return value;
}

void State::SetId(const std::string& id)
{
  this->id = id;
}

void State::SetValue(const Object::Pointer value)
{
  if (this->value != value)
  {
    const Object::Pointer oldValue(this->value);
    this->value = value;
    //fireStateChanged(oldValue);
  }
}

}
