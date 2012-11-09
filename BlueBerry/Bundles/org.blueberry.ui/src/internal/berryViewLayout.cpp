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

#include "berryViewLayout.h"

#include "berryImageDescriptor.h"

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
