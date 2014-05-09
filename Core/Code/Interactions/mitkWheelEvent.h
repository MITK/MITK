/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef WheelEvent_H_HEADER_INCLUDED_C184F366
#define WheelEvent_H_HEADER_INCLUDED_C184F366

#include <MitkCoreExports.h>
#include "mitkDisplayPositionEvent.h"
#include "mitkNumericTypes.h"


namespace mitk {

  //##Documentation
  //## @brief Event that stores coordinates and rotation on mouse wheel events
  //##
  //## Stores display position of the mouse and 3D world position in mm.
  //## @ingroup Interaction
  /**
  * \deprecatedSince{2013_03} mitk::WheelEvent is deprecated. Use mitk::MouseWheelEvent instead.
  * Refer to \see DataInteractionPage for general information about the concept of the new implementation.
  */

  class MITK_CORE_EXPORT WheelEvent : public DisplayPositionEvent
  {
  public:
    //##Documentation
    //## @brief Constructor with all necessary arguments.
    //##
    //## @param sender: the widget, that caused that event, so that it can be asked for worldCoordinates. changed later to a focus
    //## @param type, button, buttonState, key: information from the Event
    //## @param displPosition: the 2D Position e.g. from the mouse
    //## @param worldPosition: the 3D position e.g. from a picking
    //## @param delta: the movement of the mousewheel
    WheelEvent(BaseRenderer* sender, int type, int button, int buttonState, int key, const Point2D& displPosition, int delta);

    int GetDelta() const;
  protected:
    int m_Delta;
  };

} // namespace mitk



#endif /* WheelEvent_H_HEADER_INCLUDED_C184F366 */
