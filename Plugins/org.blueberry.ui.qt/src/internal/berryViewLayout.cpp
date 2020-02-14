/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryViewLayout.h"

namespace berry
{

ViewLayout::ViewLayout(PageLayout::Pointer pageLayout, ViewLayoutRec::Pointer r) :
  rec(r)
{
  poco_assert(pageLayout != 0);
  poco_assert(rec != 0);
}

bool ViewLayout::GetShowTitle()
{
  return rec->showTitle;
}

bool ViewLayout::IsCloseable()
{
  return rec->isCloseable;
}

bool ViewLayout::IsMoveable()
{
  return rec->isMoveable;
}

bool ViewLayout::IsStandalone()
{
  return rec->isStandalone;
}

void ViewLayout::SetCloseable(bool closeable)
{
  rec->isCloseable = closeable;
}

void ViewLayout::SetMoveable(bool moveable)
{
  rec->isMoveable = moveable;
}

}
