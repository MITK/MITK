/*=========================================================================

 Program:   BlueBerry Platform
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


#ifndef BERRYIDRAGOVERLISTENER_H_
#define BERRYIDRAGOVERLISTENER_H_

#include <berryMacros.h>
#include <berryMessage.h>

#include "../berryPoint.h"
#include "../berryRectangle.h"

#include "berryIDropTarget.h"

namespace berry {

/**
 * Implementers of this interface will receive notifications when objects are dragged over
 * a particular control.
 */
struct IDragOverListener : public virtual Object {

  berryObjectMacro(IDragOverListener);

  struct Events {

    typedef Message4<void*, Object::Pointer, const Point&, const Rectangle&, IDropTarget::Pointer> DragEventType;
    typedef MessageDelegate4<IDragOverListener, void*, Object::Pointer, const Point&, const Rectangle&, IDropTarget::Pointer> DragDelegate;

    DragEventType drag;

    void AddListener(IDragOverListener::Pointer listener);
    void RemoveListener(IDragOverListener::Pointer listener);

  };

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
  virtual IDropTarget::Pointer Drag(void* currentControl, Object::Pointer draggedObject,
          const Point& position, const Rectangle& dragRectangle) = 0;
};

}

#endif /* BERRYIDRAGOVERLISTENER_H_ */
