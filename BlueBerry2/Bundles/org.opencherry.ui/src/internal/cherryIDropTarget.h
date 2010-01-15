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


#ifndef CHERRYIDROPTARGET_H_
#define CHERRYIDROPTARGET_H_

#include <osgi/framework/Macros.h>

#include "../cherryRectangle.h"
#include "../tweaklets/cherryDnDTweaklet.h"

namespace cherry {

/**
 * This interface is used to drop objects. It knows how to drop a particular object
 * in a particular location. IDropTargets are typically created by IDragOverListeners, and
 * it is the job of the IDragOverListener to supply the drop target with information about
 * the object currently being dragged.
 *
 * @see IDragOverListener
 */
struct IDropTarget : public Object {

  osgiObjectMacro(IDropTarget);

  /**
   * Drops the object in this position
   */
  virtual void Drop() = 0;

  /**
   * Returns a cursor id describing this drop operation
   *
   * @return a cursor id describing this drop operation
   */
  virtual DnDTweaklet::CursorType GetCursor() = 0;

  /**
   * Returns a rectangle (screen coordinates) describing the target location
   * for this drop operation.
   *
   * @return a snap rectangle or null if this drop target does not have a specific snap
   * location.
   */
  virtual Rectangle GetSnapRectangle() = 0;

  /**
   * This is called whenever a drag operation is cancelled
   */
  virtual void DragFinished(bool dropPerformed) = 0;
};

}

#endif /* CHERRYIDROPTARGET_H_ */
