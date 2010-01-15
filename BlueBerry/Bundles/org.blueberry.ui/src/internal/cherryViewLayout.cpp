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

#include "cherryViewLayout.h"

#include "../cherryImageDescriptor.h"

namespace cherry
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
