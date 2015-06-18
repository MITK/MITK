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

#include "berryStackDropResult.h"

namespace berry
{

StackDropResult::StackDropResult(const QRect& snapRectangle,
    Object::Pointer cookie) :
  snapRectangle(snapRectangle), cookie(cookie)
{

}

QRect StackDropResult::GetSnapRectangle()
{
  return snapRectangle;
}

Object::Pointer StackDropResult::GetCookie()
{
  return cookie;
}

}
