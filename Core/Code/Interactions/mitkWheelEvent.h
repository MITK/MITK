/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:56:03 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 17179 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef WheelEvent_H_HEADER_INCLUDED_C184F366
#define WheelEvent_H_HEADER_INCLUDED_C184F366

#include "mitkCommon.h"
#include "mitkDisplayPositionEvent.h"
#include "mitkVector.h"


namespace mitk {

  //##Documentation
  //## @brief Event that stores coordinates and rotation on mouse wheel events
  //## 
  //## Stores display position of the mouse and 3D world position in mm.
  //## @ingroup Interaction
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
