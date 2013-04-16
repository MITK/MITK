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


#ifndef BERRYITRACKER_H_
#define BERRYITRACKER_H_

#include "berryDnDTweaklet.h"

#include "berryRectangle.h"
#include "guitk/berryGuiTkIControlListener.h"

namespace berry {

/**
 *  Instances of this class implement a rubber banding rectangle that is
 *  drawn onto a parent control or display.
 *  These rectangles can be specified to respond to mouse and key events
 *  by either moving or resizing themselves accordingly.  Trackers are
 *  typically used to represent window geometries in a lightweight manner.
 *
 */
struct BERRY_UI ITracker
{

  virtual ~ITracker();

  virtual Rectangle GetRectangle() = 0;
  virtual void SetRectangle(const Rectangle& rectangle) = 0;

  virtual void SetCursor(DnDTweaklet::CursorType cursor) = 0;

  virtual bool Open() = 0;

  virtual void AddControlListener(GuiTk::IControlListener::Pointer listener) = 0;
  virtual void RemoveControlListener(GuiTk::IControlListener::Pointer listener) = 0;

};

}

#endif /* BERRYITRACKER_H_ */
