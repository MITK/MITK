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


#ifndef BERRYIDROPTARGET_H_
#define BERRYIDROPTARGET_H_

#include <berryMacros.h>
#include <berryConstants.h>
#include <berryObject.h>

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

/**
 * This interface is used to drop objects. It knows how to drop a particular object
 * in a particular location. IDropTargets are typically created by IDragOverListeners, and
 * it is the job of the IDragOverListener to supply the drop target with information about
 * the object currently being dragged.
 *
 * @see IDragOverListener
 */
struct BERRY_UI_QT IDropTarget : public Object
{

  berryObjectMacro(IDropTarget)

  ~IDropTarget();

  /**
   * Drops the object in this position
   */
  virtual void Drop() = 0;

  /**
   * Returns a cursor id describing this drop operation
   *
   * @return a cursor id describing this drop operation
   */
  virtual CursorType GetCursor() = 0;

  /**
   * Returns a rectangle (screen coordinates) describing the target location
   * for this drop operation.
   *
   * @return a snap rectangle or null if this drop target does not have a specific snap
   * location.
   */
  virtual QRect GetSnapRectangle() = 0;

  /**
   * This is called whenever a drag operation is cancelled
   */
  virtual void DragFinished(bool dropPerformed) = 0;
};

}

#endif /* BERRYIDROPTARGET_H_ */
