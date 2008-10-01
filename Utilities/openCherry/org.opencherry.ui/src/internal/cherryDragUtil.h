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

#include "../cherryRectangle.h"

namespace cherry {

/**
 * Provides the methods for attaching drag-and-drop listeners to SWT controls.
 */
class DragUtil {

private:
//
//  static const std::string DROP_TARGET_ID = "org.opencherry.ui.internal.dropTarget"; //$NON-NLS-1$
//
//    /**
//     * The location where all drags will end. If this is non-null, then
//     * all user input is ignored in drag/drop. If null, we use user input
//     * to determine where objects should be dropped.
//     */
//    static TestDropLocation forcedDropTarget = null;
//
//    /**
//     * List of IDragOverListener
//     */
//    static List defaultTargets = new ArrayList();
//
//    /**
//     * Return the list of 'IDragOverListener' elements associated with
//     * the given control. If there's a 'global' listener then always
//     * return it.
//     *
//     * @param control
//     * @return
//     */
//    static List getTargetList(Control control) {
//        List result = (List) control.getData(DROP_TARGET_ID);
//        return result;
//    }
//
//    /**
//     * Given a list of IDragOverListeners and a description of what is being dragged, it returns
//     * a IDropTarget for the current drop.
//     *
//     * @param toSearch
//     * @param mostSpecificControl
//     * @param draggedObject
//     * @param position
//     * @param dragRectangle
//     * @return
//     */
//    static IDropTarget getDropTarget(List toSearch,
//            Control mostSpecificControl, Object draggedObject, Point position,
//            Rectangle dragRectangle) {
//        if (toSearch == null) {
//            return null;
//        }
//
//        Iterator iter = toSearch.iterator();
//        while (iter.hasNext()) {
//            IDragOverListener next = (IDragOverListener) iter.next();
//
//            IDropTarget dropTarget = next.drag(mostSpecificControl,
//                    draggedObject, position, dragRectangle);
//
//            if (dropTarget != null) {
//                return dropTarget;
//            }
//        }
//
//        return null;
//    }
//
//
public:
//
//    /**
//     * Sets the drop target for the given control. It is possible to add one or more
//     * targets for a "null" control. This becomes a default target that is used if no
//     * other targets are found (for example, when dragging objects off the application
//     * window).
//     *
//     * @param control the control that should be treated as a drag target, or null
//     * to indicate the default target
//     * @param target the drag target to handle the given control
//     */
//    static void addDragTarget(Control control, IDragOverListener target) {
//        if (control == null) {
//            defaultTargets.add(target);
//        } else {
//            List targetList = getTargetList(control);
//
//            if (targetList == null) {
//                targetList = new ArrayList(1);
//            }
//            targetList.add(target);
//            control.setData(DROP_TARGET_ID, targetList);
//        }
//    }
//
//
//
//    /**
//     * Removes a drop target from the given control.
//     *
//     * @param control
//     * @param target
//     */
//    static void removeDragTarget(Control control,
//            IDragOverListener target) {
//        if (control == null) {
//            defaultTargets.remove(target);
//        } else {
//            List targetList = getTargetList(control);
//            if (targetList != null) {
//                targetList.remove(target);
//                if (targetList.isEmpty()) {
//                    control.setData(DROP_TARGET_ID, null);
//                }
//            }
//        }
//    }

    /**
     * Shorthand method. Returns the bounding rectangle for the given control, in
     * display coordinates.
     *
     * @param draggedItem
     * @param boundsControl
     * @return
     */
    static Rectangle GetDisplayBounds(void* boundsControl);

//    static bool performDrag(Object::ConstPointer draggedItem,
//            const Rectangle& sourceBounds, const Point& initialLocation, bool allowSnapping) {
//
//        IDropTarget target = dragToTarget(draggedItem, sourceBounds,
//                initialLocation, allowSnapping);
//
//        if (target == null) {
//            return false;
//        }
//
//        target.drop();
//
//        // If the target can handle a 'finished' notification then send one
//        if (target!= null && target instanceof IDropTarget2) {
//          ((IDropTarget2)target).dragFinished(true);
//        }
//
//        return true;
//    }
//
//    /**
//     * Drags the given item to the given location (in display coordinates). This
//     * method is intended for use by test suites.
//     *
//     * @param draggedItem object being dragged
//     * @param finalLocation location being dragged to
//     * @return true iff the drop was accepted
//     */
//    static boolean dragTo(Display display, Object draggedItem,
//            Point finalLocation, Rectangle dragRectangle) {
//        Control currentControl = SwtUtil.findControl(display, finalLocation);
//
//        IDropTarget target = getDropTarget(currentControl, draggedItem,
//                finalLocation, dragRectangle);
//
//        if (target == null) {
//            return false;
//        }
//
//        target.drop();
//
//        return true;
//    }
//
//    /**
//     * Forces all drags to end at the given position (display coordinates). Intended
//     * for use by test suites. If this method is called, then all subsequent calls
//     * to performDrag will terminate immediately and behave as though the object were
//     * dragged to the given location. Calling this method with null cancels this
//     * behavior and causes performDrag to behave normally.
//     *
//     * @param forcedLocation location where objects will be dropped (or null to
//     * cause drag/drop to behave normally).
//     */
//    static void forceDropLocation(TestDropLocation forcedLocation) {
//        forcedDropTarget = forcedLocation;
//    }
//
//    /**
//     * Drags the given item, given an initial bounding rectangle in display coordinates.
//     * Due to a quirk in the Tracker class, changing the tracking rectangle when using the
//     * keyboard will also cause the mouse cursor to move. Since "snapping" causes the tracking
//     * rectangle to change based on the position of the mouse cursor, it is impossible to do
//     * drag-and-drop with the keyboard when snapping is enabled.
//     *
//     * @param draggedItem object being dragged
//     * @param sourceBounds initial bounding rectangle for the dragged item
//     * @param initialLocation initial position of the mouse cursor
//     * @param allowSnapping true iff the rectangle should snap to the drop location. This must
//     * be false if the user might be doing drag-and-drop using the keyboard.
//     *
//     * @return
//     */
//    static IDropTarget dragToTarget(final Object draggedItem,
//            final Rectangle sourceBounds, final Point initialLocation,
//            final boolean allowSnapping) {
//        final Display display = Display.getCurrent();
//
//        // Testing...immediately 'drop' onto the test target
//        if (forcedDropTarget != null) {
//            Point location = forcedDropTarget.getLocation();
//
//            Control currentControl = SwtUtil.findControl(forcedDropTarget.getShells(), location);
//            return getDropTarget(currentControl, draggedItem, location,
//                    sourceBounds);
//        }
//
//        // Create a tracker.  This is just an XOR rect on the screen.
//        // As it moves we notify the drag listeners.
//        final Tracker tracker = new Tracker(display, SWT.NULL);
//        tracker.setStippled(true);
//
//        tracker.addListener(SWT.Move, new Listener() {
//            public void handleEvent(final Event event) {
//                display.syncExec(new Runnable() {
//                    public void run() {
//                      // Get the curslor location as a point
//                        Point location = new Point(event.x, event.y);
//
//                        // Select a drop target; use the global one by default
//                      IDropTarget target = null;
//
//                        Control targetControl = display.getCursorControl();
//
//                        // Get the drop target for this location
//                        target = getDropTarget(targetControl,
//                                draggedItem, location,
//                                tracker.getRectangles()[0]);
//
//                      // Set up the tracker feedback based on the target
//                        Rectangle snapTarget = null;
//                        if (target != null) {
//                            snapTarget = target.getSnapRectangle();
//
//                            tracker.setCursor(target.getCursor());
//                        } else {
//                            tracker.setCursor(DragCursors
//                                    .getCursor(DragCursors.INVALID));
//                        }
//
//                        // If snapping then reset the tracker's rectangle based on the current drop target
//                        if (allowSnapping) {
//                            if (snapTarget == null) {
//                                snapTarget = new Rectangle(sourceBounds.x
//                                        + location.x - initialLocation.x,
//                                        sourceBounds.y + location.y
//                                                - initialLocation.y,
//                                        sourceBounds.width, sourceBounds.height);
//                            }
//
//                            // Try to prevent flicker: don't change the rectangles if they're already in
//                            // the right location
//                            Rectangle[] currentRectangles = tracker.getRectangles();
//
//                            if (!(currentRectangles.length == 1 && currentRectangles[0]
//                                    .equals(snapTarget))) {
//                                tracker.setRectangles(new Rectangle[] { snapTarget });
//                            }
//                        }
//                    }
//                });
//            }
//        });
//
//        // Setup...when the drag starts we might already be over a valid target, check this...
//        // If there is a 'global' target then skip the check
//        IDropTarget target = null;
//        Control startControl = display.getCursorControl();
//
//        if (startControl != null && allowSnapping) {
//            target = getDropTarget(startControl,
//                draggedItem, initialLocation,
//                sourceBounds);
//        }
//
//        // Set up an initial tracker rectangle
//        Rectangle startRect = sourceBounds;
//        if (target != null) {
//            Rectangle rect = target.getSnapRectangle();
//
//            if (rect != null) {
//                startRect = rect;
//            }
//
//            tracker.setCursor(target.getCursor());
//        }
//
//        if (startRect != null) {
//            tracker.setRectangles(new Rectangle[] { Geometry.copy(startRect)});
//        }
//
//        // Tracking Loop...tracking is preformed on the 'SWT.Move' listener registered
//        // against the tracker.
//
//        // HACK:
//        // Some control needs to capture the mouse during the drag or other
//        // controls will interfere with the cursor
//        Shell shell = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getShell();
//        if (shell != null) {
//            shell.setCapture(true);
//        }
//
//        // Run tracker until mouse up occurs or escape key pressed.
//        boolean trackingOk = tracker.open();
//
//        // HACK:
//        // Release the mouse now
//        if (shell != null) {
//            shell.setCapture(false);
//        }
//
//        // Done tracking...
//
//        // Get the current drop target
//        IDropTarget dropTarget = null;
//        Point finalLocation = display.getCursorLocation();
//        Control targetControl = display.getCursorControl();
//        dropTarget = getDropTarget(targetControl, draggedItem,
//                finalLocation, tracker.getRectangles()[0]);
//
//        // Cleanup...
//        tracker.dispose();
//
//        // if we're going to perform a 'drop' then delay the issuing of the 'finished'
//        // callback until after it's done...
//        if (trackingOk) {
//          return dropTarget;
//        }
//        else if (dropTarget!= null && dropTarget instanceof IDropTarget2) {
//            // If the target can handle a 'finished' notification then send one
//          ((IDropTarget2)dropTarget).dragFinished(false);
//        }
//
//        return null;
//    }
//
//    /**
//     * Returns the drag target for the given control or null if none.
//     *
//     * @param toSearch
//     * @param e
//     * @return
//     */
//    static IDropTarget getDropTarget(Control toSearch,
//            Object draggedObject, Point position, Rectangle dragRectangle) {
//      // Search for a listener by walking the control's parent hierarchy
//        for (Control current = toSearch; current != null; current = current
//                .getParent()) {
//            IDropTarget dropTarget = getDropTarget(getTargetList(current),
//                    toSearch, draggedObject, position, dragRectangle);
//
//            if (dropTarget != null) {
//                return dropTarget;
//            }
//
//            // Don't look to parent shells for drop targets
//            if (current instanceof Shell) {
//                break;
//            }
//        }
//
//        // No controls could handle this event -- check for default targets
//        return getDropTarget(defaultTargets, toSearch, draggedObject, position,
//                dragRectangle);
//    }
//
//    /**
//     * Returns the location of the given event, in display coordinates
//     * @return
//     */
//    static Point getEventLoc(Event event) {
//        Control ctrl = (Control) event.widget;
//        return ctrl.toDisplay(new Point(event.x, event.y));
//    }

};

}

#endif /* CHERRYDRAGUTIL_H_ */
