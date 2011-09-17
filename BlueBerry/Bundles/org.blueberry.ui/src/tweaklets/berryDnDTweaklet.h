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

#ifndef BERRYDNDTWEAKLET_H_
#define BERRYDNDTWEAKLET_H_

#include <berryMacros.h>

#include <org_blueberry_ui_Export.h>
#include "../internal/berryTweaklets.h"

namespace berry
{

struct ITracker;

/**
 * Provides the set of cursors used for drag-and-drop.
 */
struct BERRY_UI DnDTweaklet
{

  static Tweaklets::TweakKey<DnDTweaklet> KEY;

  enum CursorType
  { CURSOR_INVALID,
    CURSOR_LEFT,
    CURSOR_RIGHT,
    CURSOR_TOP,
    CURSOR_BOTTOM,
    CURSOR_CENTER,
    CURSOR_OFFSCREEN,
    CURSOR_FASTVIEW};

  static CursorType PositionToCursorType(int positionConstant);

  /**
   * Converts a DnDTweaklet::CursorType (CURSOR_LEFT, CURSOR_RIGHT, CURSOR_TOP, CURSOR_BOTTOM, CURSOR_CENTER) into a BlueBerry constant
   * (Constants::LEFT, Constants::RIGHT, Constants::TOP, Constants::BOTTOM, Constants::CENTER)
   *
   * @param dragCursorId
   * @return a BlueBerry Constants::* constant
   */
  static int CursorTypeToPosition(CursorType dragCursorId);

  virtual ITracker* CreateTracker() = 0;

};

}

Q_DECLARE_INTERFACE(berry::DnDTweaklet, "org.blueberry.DnDTweaklet")

#endif /* BERRYDNDTWEAKLET_H_ */
