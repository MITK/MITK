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

#include "mitkVtkEventAdapter.h"
#include <mitkInteractionConst.h>
#include <mitkWheelEvent.h>

#include "vtkCommand.h"

namespace mitk
{
  std::map<BaseRenderer* ,int> VtkEventAdapter::buttonStateMap;
}

mitk::MouseEvent
mitk::VtkEventAdapter::AdaptMouseEvent(mitk::BaseRenderer* sender, unsigned long vtkCommandEventId,vtkRenderWindowInteractor* rwi)
{

  mitk::Point2D p;
  p[0] = rwi->GetEventPosition()[0];
  p[1] = rwi->GetEventPosition()[1];
  p[1] = rwi->GetSize()[1] - p[1];       // flip y axis

  // http://doc.trolltech.com/4.6/qt.html#MouseButton-enum
  int button = 0;
  int type   = 0;
  int state  = 0;

  // vtkRenderWindowInteractor does not provide information about mouse buttons during
  // mouse move events...

  // some button action might be going on during mouse move events
  // that needs to be temp. saved in our buttonStateMap
  int tmpstate = 0;
  if(buttonStateMap.find(sender) != buttonStateMap.end())
    tmpstate = buttonStateMap.find(sender)->second;

  if(tmpstate != 0 && vtkCommandEventId != vtkCommand::MouseMoveEvent)
    buttonStateMap.erase(buttonStateMap.find(sender)); //press or release event with active map caching

  switch(vtkCommandEventId)
  {
    case vtkCommand::MouseMoveEvent:
      type   = 5;
      button = mitk::BS_NoButton;

      if(tmpstate != 0)
      {
        state  = tmpstate;
        /*
        // debug output
        static char  tmp;
        sprintf(&tmp,"%d",tmpstate);
        std::cout << tmp << std::endl;
        */
      }
      break;
    case vtkCommand::LeftButtonReleaseEvent:
      type   = 3;
      button = mitk::BS_LeftButton;
      state = tmpstate;

      buttonStateMap[sender] = (tmpstate - button);
      break;
    case vtkCommand::MiddleButtonReleaseEvent:
      type   = 3;
      button = mitk::BS_MidButton;
      state = tmpstate;

      buttonStateMap[sender] = (tmpstate - button);
      break;
    case vtkCommand::RightButtonReleaseEvent:
      type   = 3;
      button = mitk::BS_RightButton;
      state = tmpstate;

      buttonStateMap[sender] = (tmpstate - button);
      break;
    case vtkCommand::LeftButtonPressEvent:
      type   = 2;
      button = mitk::BS_LeftButton;

      tmpstate |= button;
      buttonStateMap[sender] = tmpstate;
      break;
    case vtkCommand::MiddleButtonPressEvent:
      type   = 2;
      button = mitk::BS_MidButton;

      tmpstate |= button;
      buttonStateMap[sender] = tmpstate;
      break;
    case vtkCommand::RightButtonPressEvent:
      type   = 2;
      button = mitk::BS_RightButton;

      tmpstate |= button;
      buttonStateMap[sender] = tmpstate;
      break;

  }

  if(rwi->GetShiftKey())
  {
    state |= mitk::BS_ShiftButton;
  }
  if(rwi->GetControlKey())
  {
    state |= mitk::BS_ControlButton;
  }
  if(rwi->GetAltKey())
  {
    state |= mitk::BS_AltButton;
  }

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


  if(rwi->GetShiftKey())
     state |= mitk::BS_ShiftButton;
  if(rwi->GetControlKey())
    state |= mitk::BS_ControlButton;
  if(rwi->GetAltKey())
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
  if(rwi->GetShiftKey())
    state |= mitk::BS_ShiftButton;
  if(rwi->GetControlKey())
     state |= mitk::BS_ControlButton;
  if(rwi->GetAltKey())
     state |= mitk::BS_AltButton;

    mitk::KeyEvent mke(sender, type, mitk::BS_NoButton, state, key, std::string(rwi->GetKeySym()), p);

  return mke;
}
