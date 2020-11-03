/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYDRAGUTIL_H_
#define BERRYDRAGUTIL_H_

#include <org_blueberry_ui_qt_Export.h>
#include "guitk/berryGuiTkIControlListener.h"

#include "berryTestDropLocation.h"
#include "berryIDragOverListener.h"
#include "berryIDropTarget.h"

#include <berryObjectList.h>

#include <QPoint>
#include <QRect>

namespace berry {

/**
 * Provides the methods for attaching drag-and-drop listeners to controls.
 */
class BERRY_UI_QT DragUtil
{

public:

  typedef ObjectList<IDragOverListener*> TargetListType;

private:

  static const QString DROP_TARGET_ID;

  /**
   * The location where all drags will end. If this is non-null, then
   * all user input is ignored in drag/drop. If null, we use user input
   * to determine where objects should be dropped.
   */
  static TestDropLocation::Pointer forcedDropTarget;

  /**
   * List of IDragOverListener
   */
  static QList<IDragOverListener*> defaultTargets;

  /**
   * Return the list of 'IDragOverListener' elements associated with
   * the given control. If there's a 'global' listener then always
   * return it.
   *
   * @param control
   * @return
   */
  static TargetListType::Pointer GetTargetList(QWidget* control);

  /**
   * Given a list of IDragOverListeners and a description of what is being dragged, it returns
   * a IDropTarget for the current drop.
   *
   * @param toSearch
   * @param mostSpecificControl
   * @param draggedObject
   * @param position
   * @param dragRectangle
   * @return
   */
  static IDropTarget::Pointer GetDropTarget(const QList<IDragOverListener*>& toSearch,
                                            QWidget* mostSpecificControl,
                                            Object::Pointer draggedObject,
                                            const QPoint &position,
                                            const QRect& dragRectangle);

public:

  /**
   * Sets the drop target for the given control. It is possible to add one or more
   * targets for a "null" control. This becomes a default target that is used if no
   * other targets are found (for example, when dragging objects off the application
   * window).
   *
   * @param control the control that should be treated as a drag target, or null
   * to indicate the default target
   * @param target the drag target to handle the given control
   */
  static void AddDragTarget(QWidget* control, IDragOverListener* target);

  /**
   * Removes a drop target from the given control.
   *
   * @param control
   * @param target
   */
  static void RemoveDragTarget(QWidget* control, IDragOverListener* target);

  /**
   * Shorthand method. Returns the bounding rectangle for the given control, in
   * display coordinates.
   *
   * @param draggedItem
   * @param boundsControl
   * @return
   */
  static QRect GetDisplayBounds(QWidget* boundsControl);

  static bool PerformDrag(Object::Pointer draggedItem,
                          const QRect& sourceBounds,
                          const QPoint& initialLocation,
                          bool allowSnapping);

//  /**
//   * Drags the given item to the given location (in display coordinates). This
//   * method is intended for use by test suites.
//   *
//   * @param draggedItem object being dragged
//   * @param finalLocation location being dragged to
//   * @return true iff the drop was accepted
//   */
//  static bool DragTo(Display display, Object draggedItem,
//          QPoint finalLocation, QRect dragRectangle) {
//      Control currentControl = SwtUtil.findControl(display, finalLocation);
//
//      IDropTarget target = getDropTarget(currentControl, draggedItem,
//              finalLocation, dragRectangle);
//
//      if (target == null) {
//          return false;
//      }
//
//      target.drop();
//
//      return true;
//  }

  /**
   * Forces all drags to end at the given position (display coordinates). Intended
   * for use by test suites. If this method is called, then all subsequent calls
   * to performDrag will terminate immediately and behave as though the object were
   * dragged to the given location. Calling this method with null cancels this
   * behavior and causes performDrag to behave normally.
   *
   * @param forcedLocation location where objects will be dropped (or null to
   * cause drag/drop to behave normally).
   */
  static void ForceDropLocation(TestDropLocation::Pointer forcedLocation);

  /**
   * Drags the given item, given an initial bounding rectangle in display coordinates.
   * Due to a quirk in the Tracker class, changing the tracking rectangle when using the
   * keyboard will also cause the mouse cursor to move. Since "snapping" causes the tracking
   * rectangle to change based on the position of the mouse cursor, it is impossible to do
   * drag-and-drop with the keyboard when snapping is enabled.
   *
   * @param draggedItem object being dragged
   * @param sourceBounds initial bounding rectangle for the dragged item
   * @param initialLocation initial position of the mouse cursor
   * @param allowSnapping true iff the rectangle should snap to the drop location. This must
   * be false if the user might be doing drag-and-drop using the keyboard.
   *
   * @return
   */
  static IDropTarget::Pointer DragToTarget(Object::Pointer draggedItem,
                                           const QRect& sourceBounds,
                                           const QPoint& initialLocation,
                                           bool allowSnapping);

  /**
   * Returns the drag target for the given control or null if none.
   *
   * @param toSearch
   * @param e
   * @return
   */
  static IDropTarget::Pointer GetDropTarget(QWidget* toSearch,
                                            Object::Pointer draggedObject,
                                            const QPoint& position,
                                            const QRect& dragRectangle);

  /**
   * Returns the location of the given event, in display coordinates
   * @return
   */
  //static QPoint GetEventLoc(GuiTk::ControlEvent::Pointer event);

};

}

#endif /* BERRYDRAGUTIL_H_ */
