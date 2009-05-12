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


#ifndef CHERRYITRACKER_H_
#define CHERRYITRACKER_H_

#include "cherryDnDTweaklet.h"

#include "../cherryRectangle.h"
#include "../guitk/cherryGuiTkIControlListener.h"

namespace cherry {

/**
 *  Instances of this class implement a rubber banding rectangle that is
 *  drawn onto a parent control or display.
 *  These rectangles can be specified to respond to mouse and key events
 *  by either moving or resizing themselves accordingly.  Trackers are
 *  typically used to represent window geometries in a lightweight manner.
 *
 */
struct ITracker
{

  virtual ~ITracker() {}
  
  virtual Rectangle GetRectangle() = 0;
  virtual void SetRectangle(const Rectangle& rectangle) = 0;

  virtual void SetCursor(DnDTweaklet::CursorType cursor) = 0;

  virtual bool Open() = 0;

  virtual void AddControlListener(GuiTk::IControlListener::Pointer listener) = 0;
  virtual void RemoveControlListener(GuiTk::IControlListener::Pointer listener) = 0;

};

}

#endif /* CHERRYITRACKER_H_ */
