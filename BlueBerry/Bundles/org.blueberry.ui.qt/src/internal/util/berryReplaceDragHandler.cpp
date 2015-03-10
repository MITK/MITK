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

#include "berryReplaceDragHandler.h"
#include "berryAbstractTabItem.h"

#include <berryGeometry.h>
#include <internal/berryDragUtil.h>

namespace berry
{

ReplaceDragHandler::DragCookie::DragCookie(int pos) :
  insertPosition(pos)
{
}

ReplaceDragHandler::ReplaceDragHandler(AbstractTabFolder* folder) :
  tabFolder(folder)
{

}

StackDropResult::Pointer ReplaceDragHandler::DragOver(QWidget*,
    const QPoint& location, int dragStart)
{

  // Determine which tab we're currently dragging over
  //Point localPos = tabFolder.getControl().toControl(location);

  AbstractTabItem* tabUnderPointer = tabFolder->GetItem(location);

  // This drop target only deals with tabs... if we're not dragging over
  // a tab, exit.
  if (tabUnderPointer == 0)
  {
    QRect titleArea = tabFolder->GetTabArea();

    // If we're dragging over the title area, treat this as a drop in the last
    // tab position.
    if (titleArea.contains(location) && tabFolder->GetItemCount() > 0)
    {
      int dragOverIndex = static_cast<int>(tabFolder->GetItemCount());
      AbstractTabItem* lastTab = tabFolder->GetItem(dragOverIndex - 1);

      // Can't drag to end unless you can see the end
      if (!lastTab->IsShowing())
      {
        return StackDropResult::Pointer(0);
      }

      // If we are unable to compute the bounds for this tab, then ignore the drop
      QRect lastTabBounds = lastTab->GetBounds();
      if (lastTabBounds.isEmpty())
      {
        return StackDropResult::Pointer(0);
      }

      if (dragStart >= 0)
      {
        dragOverIndex--;
        Object::Pointer cookie(new DragCookie(dragOverIndex));
        StackDropResult::Pointer result(new StackDropResult(lastTabBounds, cookie));
        return result;
      }

      // Make the drag-over rectangle look like a tab at the end of the tab region.
      // We don't actually know how wide the tab will be when it's dropped, so just
      // make it 3 times wider than it is tall.
      QRect dropRectangle = titleArea;

      dropRectangle.setX(lastTabBounds.x() + lastTabBounds.width());
      dropRectangle.setWidth(3 * dropRectangle.height());
      Object::Pointer cookie(new DragCookie(dragOverIndex));
      StackDropResult::Pointer result(new StackDropResult(dropRectangle, cookie));
      return result;
    }
    else
    {
      // If the closest side is the side with the tabs, consider this a stack operation.
      // Otherwise, let the drop fall through to whatever the default behavior is
      QRect displayBounds = DragUtil::GetDisplayBounds(
          tabFolder->GetControl());
      int closestSide = Geometry::GetClosestSide(displayBounds, location);
      if (closestSide == tabFolder->GetTabPosition())
      {
        StackDropResult::Pointer result(new StackDropResult(displayBounds, Object::Pointer(0)));
        return result;
      }

      return StackDropResult::Pointer(0);
    }
  }

  if (!tabUnderPointer->IsShowing())
  {
    return StackDropResult::Pointer(0);
  }

  QRect qtabBounds = tabUnderPointer->GetBounds();
  QRect tabBounds(qtabBounds.x(), qtabBounds.y(), qtabBounds.width(), qtabBounds.height());

  if (qtabBounds.isEmpty())
  {
    return StackDropResult::Pointer(0);
  }

  Object::Pointer cookie(new DragCookie(tabFolder->IndexOf(tabUnderPointer)));
  StackDropResult::Pointer result(new StackDropResult(tabBounds, cookie));
  return result;
}

int ReplaceDragHandler::GetInsertionPosition(Object::Pointer cookie)
{
  if (cookie.Cast<DragCookie>() != 0)
  {
    return std::min<int>(static_cast<int>(tabFolder->GetItemCount()),
        cookie.Cast<DragCookie>()->insertPosition);
  }

  return static_cast<int>(tabFolder->GetItemCount());
}

}
