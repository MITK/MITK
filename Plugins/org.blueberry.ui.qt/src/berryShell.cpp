/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryShell.h"

#include <QRect>

namespace berry {

Shell::Shell()
  : extraShellListener(nullptr)
{
}

Object::Pointer Shell::GetData(const QString& id) const
{
  QHash<QString, Object::Pointer>::const_iterator iter = data.find(id);
  if (iter == data.end()) return Object::Pointer(nullptr);
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
