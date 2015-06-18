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


#ifndef BERRYIDRAGOVERLISTENER_H_
#define BERRYIDRAGOVERLISTENER_H_

#include <berryMessage.h>

#include "berryObject.h"
#include "berryIDropTarget.h"

namespace berry {

/**
 * Implementers of this interface will receive notifications when objects are dragged over
 * a particular control.
 */
struct IDragOverListener
{

  struct Events {

    typedef Message4<QWidget*, const Object::Pointer&, const QPoint&, const QRect&, IDropTarget::Pointer> DragEventType;
    typedef MessageDelegate4<IDragOverListener, QWidget*, const Object::Pointer&, const QPoint&, const QRect&, IDropTarget::Pointer> DragDelegate;

    DragEventType drag;

    void AddListener(IDragOverListener* listener);
    void RemoveListener(IDragOverListener* listener);

  };

  virtual ~IDragOverListener();

  /**
   * Notifies the receiver that the given object has been dragged over
   * the given position. Returns a drop target if the object may be
   * dropped in this position. Returns null otherwise.
   *
   * @param draggedObject object being dragged over this location
   * @param position location of the cursor
   * @param dragRectangle current drag rectangle (may be an empty rectangle if none)
   * @return a valid drop target or null if none
   */
  virtual IDropTarget::Pointer Drag(QWidget* currentControl, const Object::Pointer& draggedObject,
          const QPoint& position, const QRect& dragRectangle) = 0;
};

}

#endif /* BERRYIDRAGOVERLISTENER_H_ */
