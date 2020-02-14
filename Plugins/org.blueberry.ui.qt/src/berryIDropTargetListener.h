/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYIDROPTARGETLISTENER_H
#define BERRYIDROPTARGETLISTENER_H

#include <org_blueberry_ui_qt_Export.h>

#include "berryMessage.h"

class QDragEnterEvent;
class QDragLeaveEvent;
class QDragMoveEvent;
class QDropEvent;

namespace berry {

struct BERRY_UI_QT IDropTargetListener
{

  virtual ~IDropTargetListener();

  struct Events {

    enum Type {
      NONE  = 0x00000000,
      ENTER = 0x00000001,
      LEAVE = 0x00000002,
      MOVE  = 0x00000004,
      DROP  = 0x00000008,

      ALL   = 0xffffffff
    };

    Q_DECLARE_FLAGS(Types, Type)

    typedef Message1<QDragEnterEvent*> DragEnterEventType;
    typedef Message1<QDragLeaveEvent*> DragLeaveEventType;
    typedef Message1<QDragMoveEvent*> DragMoveEventType;
    typedef Message1<QDropEvent*> DropEventType;

    DragEnterEventType dragEnter;
    DragLeaveEventType dragLeave;
    DragMoveEventType dragMove;
    DropEventType drop;

    void AddListener(IDropTargetListener* listener);
    void RemoveListener(IDropTargetListener* listener);

  private:
    typedef MessageDelegate1<IDropTargetListener, QDragEnterEvent*> DragEnterDelegate;
    typedef MessageDelegate1<IDropTargetListener, QDragLeaveEvent*> DragLeaveDelegate;
    typedef MessageDelegate1<IDropTargetListener, QDragMoveEvent*> DragMoveDelegate;
    typedef MessageDelegate1<IDropTargetListener, QDropEvent*> DropDelegate;
  };

  virtual Events::Types GetDropTargetEventTypes() const = 0;

  virtual void DragEnterEvent(QDragEnterEvent* /*event*/) {}
  virtual void DragLeaveEvent(QDragLeaveEvent* /*event*/) {}
  virtual void DragMoveEvent(QDragMoveEvent* /*event*/) {}
  virtual void DropEvent(QDropEvent* /*event*/) {}

};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(berry::IDropTargetListener::Events::Types)

#endif // BERRYIDROPTARGETLISTENER_H
