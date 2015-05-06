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
  return Object::Pointer(0);
}

void EmptyTabItem::SetData(Object::Pointer  /*data*/)
{

}

}
