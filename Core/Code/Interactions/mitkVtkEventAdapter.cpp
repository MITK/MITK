/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-08-20 14:31:52 +0200 (Do, 20 Aug 2009) $
Version:   $Revision: 18670 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkVtkEventAdapter.h"
#include <mitkInteractionConst.h>
#include <mitkWheelEvent.h>

#include "vtkCommand.h"

mitk::MouseEvent 
//mitk::VtkEventAdapter::AdaptMouseEvent(mitk::BaseRenderer* sender, QMouseEvent* mouseEvent)
mitk::VtkEventAdapter::AdaptMouseEvent(mitk::BaseRenderer* sender, unsigned long vtkCommandEventId,vtkRenderWindowInteractor* rwi)
{

  mitk::Point2D p;
  p[0] = rwi->GetEventPosition()[0];
  p[1] = rwi->GetEventPosition()[1];
  
  // http://doc.trolltech.com/4.6/qt.html#MouseButton-enum
  int button = 0;
  int type   = 0;
  int state  = 0;
  switch(vtkCommandEventId)
  {
    case vtkCommand::MouseMoveEvent:
      type   = 5;
      button = 0x00000000;
      break;
    case vtkCommand::LeftButtonReleaseEvent:
      type   = 3; 
      button = 0x00000001;
      break;
    case vtkCommand::MiddleButtonReleaseEvent:
      type   = 3;
      button = 0x00000004;
      break;
    case vtkCommand::RightButtonReleaseEvent:
      type   = 3;
      button = 0x00000002;
      break;
  }
  
  int modifiers = 0;
  if(rwi->GetShiftKey())
    modifiers |= 0x02000000;
    state |= mitk::BS_ShiftButton;
  if(rwi->GetControlKey())
    modifiers |= 0x04000000;
    state |= mitk::BS_ControlButton;
  if(rwi->GetAltKey())
    modifiers |= 0x08000000;
    state |= mitk::BS_AltButton;
 
  mitk::MouseEvent mitkEvent(sender, type, button, state, mitk::Key_none, p);
  
  return mitkEvent;
}

mitk::WheelEvent 
//mitk::VtkEventAdapter::AdaptWheelEvent(mitk::BaseRenderer* sender, QWheelEvent* wheelEvent)
mitk::VtkEventAdapter::AdaptWheelEvent(mitk::BaseRenderer* sender, unsigned long vtkCommandEventId,vtkRenderWindowInteractor* rwi)
{
  mitk::Point2D p;
  p[0] = rwi->GetEventPosition()[0];
  p[1] = rwi->GetEventPosition()[1];

  // http://doc.trolltech.com/4.6/qt.html#MouseButton-enum
  int button = 0;
  int type   = 0;
  int state  = 0;
  int delta  = 0;
  switch(vtkCommandEventId)
  {
    case vtkCommand::MouseWheelForwardEvent:
      type   = 31;          // wheel event, // see qcoreevent enum "type"
      button = 0x00000000;
      delta  = +120;         //http://doc.trolltech.com/3.3/qwheelevent.html#delta
      break;
    case vtkCommand::MouseWheelBackwardEvent:
      type   = 31; // wheel event, // see qcoreevent enum "type"
      button = 0x00000000;
      delta  = -120;         //http://doc.trolltech.com/3.3/qwheelevent.html#delta
      break;
  }
  
  int modifiers = 0;
  if(rwi->GetShiftKey())
    modifiers |= 0x02000000;
    state |= mitk::BS_ShiftButton;
  if(rwi->GetControlKey())
    modifiers |= 0x04000000;
    state |= mitk::BS_ControlButton;
  if(rwi->GetAltKey())
    modifiers |= 0x08000000;
    state |= mitk::BS_AltButton;

  mitk::WheelEvent mitkEvent(sender, type, button,state, mitk::Key_none, p, delta);

  return mitkEvent;
}


mitk::KeyEvent 
//mitk::VtkEventAdapter::AdaptKeyEvent(mitk::BaseRenderer* sender, QKeyEvent* keyEvent, const QPoint& cp)
mitk::VtkEventAdapter::AdaptKeyEvent(mitk::BaseRenderer* sender, unsigned long vtkCommandEventId,vtkRenderWindowInteractor* rwi)
{
  // TODO: is this viable?
  int key = (int)rwi->GetKeyCode();
  
  // Those keycodes changed in Qt 4
  if (key >= 0x01000000 && key <= 0x01000060)
    key -= (0x01000000 - 0x1000);
  else if(key >= 0x01001120 && key <= 0x01001262)
    key -= 0x01000000;

  mitk::Point2D p;
  p[0] = rwi->GetEventPosition()[0];
  p[1] = rwi->GetEventPosition()[1];

  int type = 0;
  if(vtkCommandEventId == vtkCommand::KeyPressEvent)
    type = 6; // http://doc.trolltech.com/4.6/qevent.html#Type-enum
  
  int state = 0;
  int modifiers = 0;
  if(rwi->GetShiftKey())
    modifiers |= 0x02000000;
    state |= mitk::BS_ShiftButton;
  if(rwi->GetControlKey())
    modifiers |= 0x04000000;
    state |= mitk::BS_ControlButton;
  if(rwi->GetAltKey())
    modifiers |= 0x08000000;
    state |= mitk::BS_AltButton;

    mitk::KeyEvent mke(sender, type, mitk::BS_NoButton, state, key, std::string(rwi->GetKeySym()), p);
  
  return mke;
}
