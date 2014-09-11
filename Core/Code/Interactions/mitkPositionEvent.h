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


#ifndef POSITIONEVENT_H_HEADER_INCLUDED_C184F366
#define POSITIONEVENT_H_HEADER_INCLUDED_C184F366

#include <MitkCoreExports.h>
#include "mitkDisplayPositionEvent.h"
#include "mitkNumericTypes.h"


namespace mitk {

  //##Documentation
  //## @brief Event that stores coordinates
  //##
  //## Stores display position of the mouse and 3D world position in mm.
  //## @ingroup Interaction

  /**
    * \deprecatedSince{2013_03} mitk::PositionEvent is deprecated. Use mitk::InteractionPositionEvent instead.
    * Refer to \see DataInteractionPage  for general information about the concept of the new implementation.
    */
  class MITK_CORE_EXPORT PositionEvent : public DisplayPositionEvent
  {
  public:
    //##Documentation
    //## @brief Constructor with all necessary arguments.
    //##
    //## @param sender: the widget, that caused that event, so that it can be asked for worldCoordinates. changed later to a focus
    //## @param type, button, buttonState, key: information from the Event
    //## @param displPosition: the 2D Position e.g. from the mouse
    //## @param worldPosition: the 3D position e.g. from a picking
    PositionEvent(BaseRenderer* sender, int type, int button, int buttonState, int key, const Point2D& displPosition, const Point3D& worldPosition);
  };

} // namespace mitk



#endif /* POSITIONEVENT_H_HEADER_INCLUDED_C184F366 */
