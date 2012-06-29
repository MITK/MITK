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


#include "berryIDropTargetListener.h"

namespace berry {

void IDropTargetListener::Events::AddListener(IDropTargetListener* l)
{
  if (l == 0) return;

  Types t = l->GetDropTargetEventTypes();

  if (t & ENTER)
    dragEnter += DragEnterDelegate(l, &IDropTargetListener::DragEnterEvent);
  if (t & LEAVE)
    dragLeave += DragLeaveDelegate(l, &IDropTargetListener::DragLeaveEvent);
  if (t & MOVE)
    dragMove += DragMoveDelegate(l, &IDropTargetListener::DragMoveEvent);
  if (t & DROP)
    drop += DropDelegate(l, &IDropTargetListener::DropEvent);
}

void IDropTargetListener::Events::RemoveListener(IDropTargetListener* l)
{
  if (l == 0) return;

  dragEnter -= DragEnterDelegate(l, &IDropTargetListener::DragEnterEvent);
  dragLeave -= DragLeaveDelegate(l, &IDropTargetListener::DragLeaveEvent);
  dragMove -= DragMoveDelegate(l, &IDropTargetListener::DragMoveEvent);
  drop -= DropDelegate(l, &IDropTargetListener::DropEvent);
}

}
