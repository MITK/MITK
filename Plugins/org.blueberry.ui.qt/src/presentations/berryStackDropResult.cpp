/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
