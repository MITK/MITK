/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryEmptyTabItem.h"

namespace berry
{

QRect EmptyTabItem::GetBounds()
{
  return QRect();
}

void EmptyTabItem::SetInfo(const PartInfo&  /*info*/)
{

}

void EmptyTabItem::Dispose()
{

}

Object::Pointer EmptyTabItem::GetData()
{
  return Object::Pointer(nullptr);
}

void EmptyTabItem::SetData(Object::Pointer  /*data*/)
{

}

}
