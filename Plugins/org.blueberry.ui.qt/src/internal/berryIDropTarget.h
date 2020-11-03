/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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

  berryObjectMacro(IDropTarget);

  ~IDropTarget() override;

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
