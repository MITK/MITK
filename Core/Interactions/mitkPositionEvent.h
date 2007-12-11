/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#ifndef POSITIONEVENT_H_HEADER_INCLUDED_C184F366
#define POSITIONEVENT_H_HEADER_INCLUDED_C184F366

#include "mitkCommon.h"
#include "mitkDisplayPositionEvent.h"
#include "mitkVector.h"


namespace mitk {

  //##ModelId=3E5B79B5012A
  //##Documentation
  //## @brief Event that stores coordinates
  //## 
  //## Stores display position of the mouse and 3D world position in mm.
  //## @ingroup Interaction
  class PositionEvent : public DisplayPositionEvent
  {
  public:
    //##ModelId=3E5B7CF900F0
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
