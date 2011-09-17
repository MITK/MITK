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

#include "berryDnDTweaklet.h"

#include "../berryConstants.h"

namespace berry
{

Tweaklets::TweakKey<DnDTweaklet> DnDTweaklet::KEY =
  Tweaklets::TweakKey<DnDTweaklet>();

DnDTweaklet::CursorType DnDTweaklet::PositionToCursorType(int positionConstant)
{
  if (positionConstant == Constants::LEFT)
    return CURSOR_LEFT;
  if (positionConstant == Constants::RIGHT)
    return CURSOR_RIGHT;
  if (positionConstant == Constants::TOP)
    return CURSOR_TOP;
  if (positionConstant == Constants::BOTTOM)
    return CURSOR_BOTTOM;
  if (positionConstant == Constants::CENTER)
    return CURSOR_CENTER;


  return CURSOR_INVALID;
}

int DnDTweaklet::CursorTypeToPosition(CursorType dragCursorId)
{
  switch (dragCursorId)
  {
  case CURSOR_LEFT:
    return Constants::LEFT;
  case CURSOR_RIGHT:
    return Constants::RIGHT;
  case CURSOR_TOP:
    return Constants::TOP;
  case CURSOR_BOTTOM:
    return Constants::BOTTOM;
  case CURSOR_CENTER:
    return Constants::CENTER;
  default:
    return Constants::DEFAULT;
  }
}

}

