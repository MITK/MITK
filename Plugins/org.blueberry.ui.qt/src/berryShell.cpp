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

#include <QRect>

namespace berry {

Shell::Shell()
  : extraShellListener(NULL)
{
}

Object::Pointer Shell::GetData(const QString& id) const
{
  QHash<QString, Object::Pointer>::const_iterator iter = data.find(id);
  if (iter == data.end()) return Object::Pointer(0);
  return iter.value();
}

void Shell::SetData(const Object::Pointer& data, const QString& id)
{
  this->data[id] = data;
}

IShellListener* Shell::GetExtraShellListener() const
{
  return extraShellListener;
}

void Shell::SetExtraShellListener(IShellListener* l)
{
  extraShellListener = l;
}

void Shell::SetBounds(int x, int y, int width, int height)
{
  QRect rect(x, y, width, height);
  this->SetBounds(rect);
}

}
