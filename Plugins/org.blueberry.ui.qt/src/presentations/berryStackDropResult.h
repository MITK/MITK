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


#ifndef BERRYSTACKDROPRESULT_H_
#define BERRYSTACKDROPRESULT_H_

#include <berryMacros.h>
#include <berryObject.h>

#include <org_blueberry_ui_qt_Export.h>

#include <QRect>

namespace berry {

/**
 * This structure describes a drop event that will cause a dragged part
 * to be stacked in a position currently occupied by another part.
 *
 */
class BERRY_UI_QT StackDropResult : public Object {

public:

  berryObjectMacro(StackDropResult)

private:

  QRect snapRectangle;
  Object::Pointer cookie;

public:

  /**
   * Creates a drop result
   *
   * @param snapRectangle region that should be highlighted by the tracking
   * rectangle (display coordinates)
   * @param cookie the presentation may attach an object to this drop result
   * in order to identify the drop location. This object will be passed back into the
   * presentation's add method.
   */
  StackDropResult(const QRect& snapRectangle, Object::Pointer cookie);

  /**
   * Returns a rectangle (screen coordinates) describing the target location
   * for this drop operation. While dragging, the tracking rectangle will
   * snap to this position.
   *
   * @return a snap rectangle (not null)
   */
  QRect GetSnapRectangle();

  /**
   * Returns the cookie for this drop result. This object provided by the presentation,
   * but is remembered by the workbench. It will be given back to the presentation's add
   * method to indicate that a part is being added as a result of a drop operation.
   *
   * @return the drop cookie for this drop result
   */
  Object::Pointer GetCookie();

};

}

#endif /* BERRYSTACKDROPRESULT_H_ */
