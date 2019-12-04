/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "berryIDropTargetListener.h"

namespace berry {

IDropTargetListener::~IDropTargetListener()
{
}

void IDropTargetListener::Events::AddListener(IDropTargetListener* l)
{
  if (l == nullptr) return;

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
  if (l == nullptr) return;

  dragEnter -= DragEnterDelegate(l, &IDropTargetListener::DragEnterEvent);
  dragLeave -= DragLeaveDelegate(l, &IDropTargetListener::DragLeaveEvent);
  dragMove -= DragMoveDelegate(l, &IDropTargetListener::DragMoveEvent);
  drop -= DropDelegate(l, &IDropTargetListener::DropEvent);
}

}
