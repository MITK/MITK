/*=========================================================================

 Program:   BlueBerry Platform
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
