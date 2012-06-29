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

#include "berryShell.h"

namespace berry {

Object::Pointer Shell::GetData(const std::string& id) const
{
  std::map<std::string, Object::Pointer>::const_iterator iter = data.find(id);
  if (iter == data.end()) return Object::Pointer(0);
  return iter->second;
}

void Shell::SetData(Object::Pointer data, const std::string& id)
{
  this->data[id] = data;
}

void Shell::SetBounds(int x, int y, int width, int height)
{
  Rectangle rect(x, y, width, height);
  this->SetBounds(rect);
}

}
