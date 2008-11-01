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

#include "cherryDragUtil.h"

#include "../cherryGeometry.h"
#include "../tweaklets/cherryGuiWidgetsTweaklet.h"
#include "../tweaklets/cherryDnDTweaklet.h"
#include "../tweaklets/cherryITracker.h"

namespace cherry
{

const std::string DragUtil::DROP_TARGET_ID =
    "org.opencherry.ui.internal.dropTarget";

TestDropLocation::Pointer DragUtil::forcedDropTarget = 0;

std::list<IDragOverListener::Pointer> DragUtil::defaultTargets =
    std::list<IDragOverListener::Pointer>();

DragUtil::TrackerMoveListener::TrackerMoveListener(Object::Pointer draggedItem, const Rectangle& sourceBounds,
    const Point& initialLocation, bool allowSnapping)
 : allowSnapping(allowSnapping), draggedItem(draggedItem), sourceBounds(sourceBounds), initialLocation(initialLocation)
 {

 }

void DragUtil::TrackerMoveListener::ControlMoved(
    GuiTk::ControlEvent::Pointer event)
{

  // Get the curslor location as a point
  Point location(event->x, event->y);

  // Select a drop target; use the global one by default
  IDropTarget::Pointer target;

  void* targetControl = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetCursorControl();

  // Get the ITracker which fired the event
  ITracker* tracker = static_cast<ITracker*>(event->item);

  // Get the drop target for this location
  target = DragUtil::GetDropTarget(targetControl, draggedItem, location,
      tracker->GetRectangle());

  // Set up the tracker feedback based on the target
  Rectangle snapTarget;
  if (target != 0)
  {
    snapTarget = target->GetSnapRectangle();

    tracker->SetCursor(target->GetCursor());
  }
  else
  {
    tracker->SetCursor(DnDTweaklet::CURSOR_INVALID);
  }

  // If snapping then reset the tracker's rectangle based on the current drop target
  if (allowSnapping)
  {
    if (snapTarget.width == 0 || snapTarget.height == 0)
    {
      snapTarget
          = Rectangle(sourceBounds.x + location.x - initialLocation.x, sourceBounds.y
              + location.y - initialLocation.y, sourceBounds.width, sourceBounds.height);
    }

    // Try to prevent flicker: don't change the rectangles if they're already in
    // the right location
    Rectangle currentRectangle = tracker->GetRectangle();

    if (!(currentRectangle == snapTarget))
    {
      tracker->SetRectangle(snapTarget);
    }
  }

}

DragUtil::TargetListType::Pointer DragUtil::GetTargetList(void* control)
{
  Object::Pointer data = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetData(
      control, DROP_TARGET_ID);
  TargetListType::Pointer list = data.Cast<TargetListType> ();
  return list;
}

IDropTarget::Pointer DragUtil::GetDropTarget(
    const std::list<IDragOverListener::Pointer>& toSearch,
    void* mostSpecificControl, Object::Pointer draggedObject,
    const Point& position, const Rectangle& dragRectangle)
{

  for (std::list<IDragOverListener::Pointer>::const_iterator iter =
      toSearch.begin(); iter != toSearch.end(); ++iter)
  {
    IDragOverListener::Pointer next = *iter;

    IDropTarget::Pointer dropTarget = next->Drag(mostSpecificControl,
        draggedObject, position, dragRectangle);

    if (dropTarget != 0)
    {
      return dropTarget;
    }
  }

  return 0;
}

void DragUtil::AddDragTarget(void* control, IDragOverListener::Pointer target)
{
  if (control == 0)
  {
    defaultTargets.push_back(target);
  }
  else
  {
    TargetListType::Pointer targetList = GetTargetList(control);

    if (targetList == 0)
    {
      targetList = new TargetListType();
      Tweaklets::Get(GuiWidgetsTweaklet::KEY)->SetData(control, DROP_TARGET_ID,
          targetList);
    }
    targetList->push_back(target);
  }
}

void DragUtil::RemoveDragTarget(void* control,
    IDragOverListener::Pointer target)
{
  if (control == 0)
  {
    defaultTargets.push_back(target);
  }
  else
  {
    TargetListType::Pointer targetList = GetTargetList(control);
    if (targetList != 0)
    {
      targetList->remove(target);
      if (targetList->empty())
      {
        Tweaklets::Get(GuiWidgetsTweaklet::KEY)->SetData(control,
            DROP_TARGET_ID, 0);
      }
    }
  }
}

Rectangle DragUtil::GetDisplayBounds(void* boundsControl)
{
  void* parent = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetParent(
      boundsControl);
  if (parent == 0)
  {
    return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetBounds(boundsControl);
  }

  Rectangle rect = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetBounds(
      boundsControl);
  return Geometry::ToDisplay(parent, rect);
}

bool DragUtil::PerformDrag(Object::Pointer draggedItem,
    const Rectangle& sourceBounds, const Point& initialLocation,
    bool allowSnapping)
{

  IDropTarget::Pointer target = DragToTarget(draggedItem, sourceBounds,
      initialLocation, allowSnapping);

  if (target == 0)
  {
    return false;
  }

  target->Drop();
  target->DragFinished(true);

  return true;
}

void DragUtil::ForceDropLocation(TestDropLocation::Pointer forcedLocation)
{
  forcedDropTarget = forcedLocation;
}

IDropTarget::Pointer DragUtil::DragToTarget(Object::Pointer draggedItem,
    const Rectangle& sourceBounds, const Point& initialLocation,
    bool allowSnapping)
{
  //final Display display = Display.getCurrent();

  // Testing...immediately 'drop' onto the test target
  if (forcedDropTarget != 0)
  {
    Point location = forcedDropTarget->GetLocation();

    void* currentControl =
        Tweaklets::Get(GuiWidgetsTweaklet::KEY)->FindControl(
            forcedDropTarget->GetShells(), location);
    return GetDropTarget(currentControl, draggedItem, location, sourceBounds);
  }

  // Create a tracker.  This is just an XOR rect on the screen.
  // As it moves we notify the drag listeners.
  ITracker* tracker = Tweaklets::Get(DnDTweaklet::KEY)->CreateTracker();
  //tracker.setStippled(true);

  GuiTk::IControlListener::Pointer trackerListener = new TrackerMoveListener(draggedItem,
      sourceBounds, initialLocation, allowSnapping);

  tracker->AddControlListener(trackerListener);

  // Setup...when the drag starts we might already be over a valid target, check this...
  // If there is a 'global' target then skip the check
  IDropTarget::Pointer target;
  void* startControl =
      Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetCursorControl();

  if (startControl != 0 && allowSnapping)
  {
    target = GetDropTarget(startControl, draggedItem, initialLocation,
        sourceBounds);
  }

  // Set up an initial tracker rectangle
  Rectangle startRect = sourceBounds;
  if (target != 0)
  {
    Rectangle rect = target->GetSnapRectangle();

    if (rect.width != 0 && rect.height != 0)
    {
      startRect = rect;
    }

    tracker->SetCursor(target->GetCursor());
  }

  if (startRect.width != 0 && startRect.height != 0)
  {
    tracker->SetRectangle(startRect);
  }

  // Tracking Loop...tracking is preformed on the 'SWT.Move' listener registered
  // against the tracker.

//  // HACK:
//  // Some control needs to capture the mouse during the drag or other
//  // controls will interfere with the cursor
//  Shell shell = PlatformUI.getWorkbench().getActiveWorkbenchWindow().getShell();
//  if (shell != null)
//  {
//    shell.setCapture(true);
//  }

  // Run tracker until mouse up occurs or escape key pressed.
  bool trackingOk = tracker->Open();

//  // HACK:
//  // Release the mouse now
//  if (shell != null)
//  {
//    shell.setCapture(false);
//  }

  // Done tracking...

  // Get the current drop target
  IDropTarget::Pointer dropTarget;
  Point finalLocation = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetCursorLocation();
  void* targetControl = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetCursorControl();
  dropTarget = GetDropTarget(targetControl, draggedItem, finalLocation,
      tracker->GetRectangle());

  // Cleanup...
  delete tracker;

  // if we're going to perform a 'drop' then delay the issuing of the 'finished'
  // callback until after it's done...
  if (trackingOk)
  {
    return dropTarget;
  }
  else if (dropTarget != 0)
  {
    // If the target can handle a 'finished' notification then send one
    dropTarget->DragFinished(false);
  }

  return 0;
}

IDropTarget::Pointer DragUtil::GetDropTarget(void* toSearch,
    Object::Pointer draggedObject, const Point& position,
    const Rectangle& dragRectangle)
{
  // Search for a listener by walking the control's parent hierarchy
  for (void* current = toSearch; current != 0; current = Tweaklets::Get(
      GuiWidgetsTweaklet::KEY)->GetParent(current))
  {
    TargetListType::Pointer targetList = GetTargetList(current);
    std::list<IDragOverListener::Pointer> targets;
    if (targetList != 0)
      targets.assign(targetList->begin(), targetList->end());

    IDropTarget::Pointer dropTarget = GetDropTarget(targets,
        toSearch, draggedObject, position, dragRectangle);

    if (dropTarget != 0)
    {
      return dropTarget;
    }

    //          // Don't look to parent shells for drop targets
    //          if (current instanceof Shell) {
    //              break;
    //          }
  }

  // No controls could handle this event -- check for default targets
  return GetDropTarget(defaultTargets, toSearch, draggedObject, position,
      dragRectangle);
}

//Point DragUtil::GetEventLoc(GuiTk::ControlEvent::Pointer event)
//{
//  Control ctrl = (Control) event.widget;
//  return ctrl.toDisplay(new Point(event.x, event.y));
//}

}
