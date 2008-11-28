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


#ifndef CHERRYDRAGUTIL_H_
#define CHERRYDRAGUTIL_H_

#include "../cherryUiDll.h"
#include "../cherryRectangle.h"
#include "../guitk/cherryGuiTkIControlListener.h"

#include "cherryTestDropLocation.h"
#include "cherryIDragOverListener.h"
#include "cherryIDropTarget.h"

#include <cherryObjectList.h>
#include <list>

namespace cherry {

/**
 * Provides the methods for attaching drag-and-drop listeners to SWT controls.
 */
class CHERRY_UI DragUtil {

public:

  typedef ObjectList<IDragOverListener::Pointer> TargetListType;

private:

  static const std::string DROP_TARGET_ID; //$NON-NLS-1$

    /**
     * The location where all drags will end. If this is non-null, then
     * all user input is ignored in drag/drop. If null, we use user input
     * to determine where objects should be dropped.
     */
    static TestDropLocation::Pointer forcedDropTarget;

    /**
     * List of IDragOverListener
     */
    static std::list<IDragOverListener::Pointer> defaultTargets;

    /**
     * Return the list of 'IDragOverListener' elements associated with
     * the given control. If there's a 'global' listener then always
     * return it.
     *
     * @param control
     * @return
     */
    static TargetListType::Pointer GetTargetList(void* control);

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
    static IDropTarget::Pointer GetDropTarget(const std::list<IDragOverListener::Pointer>& toSearch,
            void* mostSpecificControl, Object::Pointer draggedObject, const Point &position,
            const Rectangle& dragRectangle);


    struct TrackerMoveListener : public GuiTk::IControlListener {

      TrackerMoveListener(Object::Pointer draggedItem, const Rectangle& sourceBounds,
          const Point& initialLocation, bool allowSnapping);

      void ControlMoved(GuiTk::ControlEvent::Pointer event);

    private:

      bool allowSnapping;
      Object::Pointer draggedItem;
      Rectangle sourceBounds;
      Point initialLocation;
    };


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
    static void AddDragTarget(void* control, IDragOverListener::Pointer target);

    /**
     * Removes a drop target from the given control.
     *
     * @param control
     * @param target
     */
    static void RemoveDragTarget(void* control,
            IDragOverListener::Pointer target);

    /**
     * Shorthand method. Returns the bounding rectangle for the given control, in
     * display coordinates.
     *
     * @param draggedItem
     * @param boundsControl
     * @return
     */
    static Rectangle GetDisplayBounds(void* boundsControl);

  static bool PerformDrag(Object::Pointer draggedItem,
          const Rectangle& sourceBounds, const Point& initialLocation, bool allowSnapping);

//  /**
//   * Drags the given item to the given location (in display coordinates). This
//   * method is intended for use by test suites.
//   *
//   * @param draggedItem object being dragged
//   * @param finalLocation location being dragged to
//   * @return true iff the drop was accepted
//   */
//  static bool DragTo(Display display, Object draggedItem,
//          Point finalLocation, Rectangle dragRectangle) {
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
          const Rectangle& sourceBounds, const Point& initialLocation,
          bool allowSnapping);

  /**
   * Returns the drag target for the given control or null if none.
   *
   * @param toSearch
   * @param e
   * @return
   */
  static IDropTarget::Pointer GetDropTarget(void* toSearch,
          Object::Pointer draggedObject, const Point& position, const Rectangle& dragRectangle);

  /**
   * Returns the location of the given event, in display coordinates
   * @return
   */
  //static Point GetEventLoc(GuiTk::ControlEvent::Pointer event);

};

}

#endif /* CHERRYDRAGUTIL_H_ */
