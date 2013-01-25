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
#include <mitkInteractionConst.h> // INTERACTION LEGACY
#include <mitkWheelEvent.h>// INTERACTION LEGACY
#include "mitkInteractionEventConst.h"
#include <string.h>
#include "vtkCommand.h"

namespace mitk
{
  std::map<BaseRenderer*, int> VtkEventAdapter::buttonStateMap;
}

mitk::MouseEvent mitk::VtkEventAdapter::AdaptMouseEvent(mitk::BaseRenderer* sender,
    unsigned long vtkCommandEventId,
    vtkRenderWindowInteractor* rwi)
{

  mitk::Point2D p;
  p[0] = rwi->GetEventPosition()[0];
  p[1] = rwi->GetEventPosition()[1];
  p[1] = rwi->GetSize()[1] - p[1];       // flip y axis

  // http://doc.trolltech.com/4.6/qt.html#MouseButton-enum
  int button = 0;
  int type = 0;
  int state = 0;

  // vtkRenderWindowInteractor does not provide information about mouse buttons during
  // mouse move events...

  // some button action might be going on during mouse move events
  // that needs to be temp. saved in our buttonStateMap
  int tmpstate = 0;
  if (buttonStateMap.find(sender) != buttonStateMap.end())
    tmpstate = buttonStateMap.find(sender)->second;

  if (tmpstate != 0 && vtkCommandEventId != vtkCommand::MouseMoveEvent)
    buttonStateMap.erase(buttonStateMap.find(sender)); //press or release event with active map caching

  switch (vtkCommandEventId)
  {
  case vtkCommand::MouseMoveEvent:
    type = 5;
    button = mitk::BS_NoButton;

    if (tmpstate != 0)
    {
      state = tmpstate;
      /*
       // debug output
       static char  tmp;
       sprintf(&tmp,"%d",tmpstate);
       std::cout << tmp << std::endl;
       */
    }
    break;
  case vtkCommand::LeftButtonReleaseEvent:
    type = 3;
    button = mitk::BS_LeftButton;
    state = tmpstate;

    buttonStateMap[sender] = (tmpstate - button);
    break;
  case vtkCommand::MiddleButtonReleaseEvent:
    type = 3;
    button = mitk::BS_MidButton;
    state = tmpstate;

    buttonStateMap[sender] = (tmpstate - button);
    break;
  case vtkCommand::RightButtonReleaseEvent:
    type = 3;
    button = mitk::BS_RightButton;
    state = tmpstate;

    buttonStateMap[sender] = (tmpstate - button);
    break;
  case vtkCommand::LeftButtonPressEvent:
    type = 2;
    button = mitk::BS_LeftButton;

    tmpstate |= button;
    buttonStateMap[sender] = tmpstate;
    break;
  case vtkCommand::MiddleButtonPressEvent:
    type = 2;
    button = mitk::BS_MidButton;

    tmpstate |= button;
    buttonStateMap[sender] = tmpstate;
    break;
  case vtkCommand::RightButtonPressEvent:
    type = 2;
    button = mitk::BS_RightButton;

    tmpstate |= button;
    buttonStateMap[sender] = tmpstate;
    break;

  }

  if (rwi->GetShiftKey())
  {
    state |= mitk::BS_ShiftButton;
  }
  if (rwi->GetControlKey())
  {
    state |= mitk::BS_ControlButton;
  }
  if (rwi->GetAltKey())
  {
    state |= mitk::BS_AltButton;
  }

  mitk::MouseEvent mitkEvent(sender, type, button, state, mitk::Key_none, p);

  return mitkEvent;
}

mitk::WheelEvent
//mitk::VtkEventAdapter::AdaptWheelEvent(mitk::BaseRenderer* sender, QWheelEvent* wheelEvent)
mitk::VtkEventAdapter::AdaptWheelEvent(mitk::BaseRenderer* sender, unsigned long vtkCommandEventId, vtkRenderWindowInteractor* rwi)
{
  mitk::Point2D p;
  p[0] = rwi->GetEventPosition()[0];
  p[1] = rwi->GetEventPosition()[1];

  // http://doc.trolltech.com/4.6/qt.html#MouseButton-enum
  int button = 0;
  int type = 0;
  int state = 0;
  int delta = 0;
  switch (vtkCommandEventId)
  {
  case vtkCommand::MouseWheelForwardEvent:
    type = 31;          // wheel event, // see qcoreevent enum "type"
    button = 0x00000000;
    delta = +120;         //http://doc.trolltech.com/3.3/qwheelevent.html#delta
    break;
  case vtkCommand::MouseWheelBackwardEvent:
    type = 31; // wheel event, // see qcoreevent enum "type"
    button = 0x00000000;
    delta = -120;         //http://doc.trolltech.com/3.3/qwheelevent.html#delta
    break;
  }

  if (rwi->GetShiftKey())
    state |= mitk::BS_ShiftButton;
  if (rwi->GetControlKey())
    state |= mitk::BS_ControlButton;
  if (rwi->GetAltKey())
    state |= mitk::BS_AltButton;

  mitk::WheelEvent mitkEvent(sender, type, button, state, mitk::Key_none, p, delta);

  return mitkEvent;
}

mitk::KeyEvent
//mitk::VtkEventAdapter::AdaptKeyEvent(mitk::BaseRenderer* sender, QKeyEvent* keyEvent, const QPoint& cp)
mitk::VtkEventAdapter::AdaptKeyEvent(mitk::BaseRenderer* sender, unsigned long vtkCommandEventId, vtkRenderWindowInteractor* rwi)
{
  // TODO: is this viable?
  int key = (int) rwi->GetKeyCode();

  // Those keycodes changed in Qt 4
  if (key >= 0x01000000 && key <= 0x01000060)
    key -= (0x01000000 - 0x1000);
  else if (key >= 0x01001120 && key <= 0x01001262)
    key -= 0x01000000;

  mitk::Point2D p;
  p[0] = rwi->GetEventPosition()[0];
  p[1] = rwi->GetEventPosition()[1];

  int type = 0;
  if (vtkCommandEventId == vtkCommand::KeyPressEvent)
    type = 6; // http://doc.trolltech.com/4.6/qevent.html#Type-enum

  int state = 0;
  if (rwi->GetShiftKey())
    state |= mitk::BS_ShiftButton;
  if (rwi->GetControlKey())
    state |= mitk::BS_ControlButton;
  if (rwi->GetAltKey())
    state |= mitk::BS_AltButton;

  mitk::KeyEvent mke(sender, type, mitk::BS_NoButton, state, key, std::string(rwi->GetKeySym()), p);

  return mke;
}

mitk::MousePressEvent::Pointer mitk::VtkEventAdapter::AdaptMousePressEvent(mitk::BaseRenderer* sender,
    unsigned long vtkCommandEventId,
    vtkRenderWindowInteractor* rwi)
{
  mitk::Point2D point;
  point[0] = rwi->GetEventPosition()[0];
  point[1] = rwi->GetEventPosition()[1];
  point[1] = rwi->GetSize()[1] - point[1]; // flip y axis

  int button = 0;
  int modifiers = 0;
  int buttonState = 0;

  // vtkRenderWindowInteractor does not provide information about mouse buttons during
  // mouse move events therefore the Press/Release functions update the button state in the buttonStateMap
  // this button state is then provided for mouse move events

  if (buttonStateMap.find(sender) != buttonStateMap.end())
    buttonState = buttonStateMap.find(sender)->second;

  if (buttonState != 0 && vtkCommandEventId != vtkCommand::MouseMoveEvent)
    buttonStateMap.erase(buttonStateMap.find(sender)); //press or release event with active map caching

  switch (vtkCommandEventId)
  {
  case vtkCommand::LeftButtonPressEvent:
    button = mitk::LeftMouseButton;
    buttonState |= button;
    buttonStateMap[sender] = buttonState;
    break;
  case vtkCommand::MiddleButtonPressEvent:
    button = mitk::MiddleMouseButton;
    buttonState |= button;
    buttonStateMap[sender] = buttonState;
    break;
  case vtkCommand::RightButtonPressEvent:
    button = mitk::RightMouseButton;
    buttonState |= button;
    buttonStateMap[sender] = buttonState;
    break;
  }

  if (rwi->GetShiftKey())
  {
    modifiers |= mitk::ShiftKey;
  }
  if (rwi->GetControlKey())
  {
    modifiers |= mitk::ControlKey;
  }
  if (rwi->GetAltKey())
  {
    modifiers |= mitk::AltKey;
  }

  MousePressEvent::Pointer mpe = MousePressEvent::New(sender, point, (MouseButtons) buttonState, (ModifierKeys) modifiers,
      (MouseButtons) button);
  return mpe;
}

mitk::MouseMoveEvent::Pointer mitk::VtkEventAdapter::AdaptMouseMoveEvent(mitk::BaseRenderer* sender,
    unsigned long vtkCommandEventId,
    vtkRenderWindowInteractor* rwi)
{
  mitk::Point2D point;
  point[0] = rwi->GetEventPosition()[0];
  point[1] = rwi->GetEventPosition()[1];
  point[1] = rwi->GetSize()[1] - point[1]; // flip y axis

  int modifiers = 0;

  // vtkRenderWindowInteractor does not provide information about mouse buttons during
  // mouse move events therefore the Press/Release functions update the button state in the buttonStateMap
  // this button state is then provided for mouse move events

  int buttonState = 0;

  if (buttonStateMap.find(sender) != buttonStateMap.end())
  {
    // set stored button state
    buttonState = buttonStateMap.find(sender)->second;
  }

  if (vtkCommandEventId == vtkCommand::MouseMoveEvent)
  {
    if (buttonState != 0)
    {
      modifiers = buttonState;
    }
  }
  else
  {
    MITK_WARN<< "Wrong usage of function AdaptMouseMoveEvent. Wrong input type.";
  }

  if (rwi->GetShiftKey())
  {
    modifiers |= mitk::ShiftKey;
  }
  if (rwi->GetControlKey())
  {
    modifiers |= mitk::ControlKey;
  }
  if (rwi->GetAltKey())
  {
    modifiers |= mitk::AltKey;
  }

  MouseMoveEvent::Pointer mme = MouseMoveEvent::New(sender, point, (MouseButtons) buttonState, (ModifierKeys) modifiers);
  return mme;
}

mitk::MouseReleaseEvent::Pointer mitk::VtkEventAdapter::AdaptMouseReleaseEvent(mitk::BaseRenderer* sender,
    unsigned long vtkCommandEventId,
    vtkRenderWindowInteractor* rwi)
{
  mitk::Point2D point;
  point[0] = rwi->GetEventPosition()[0];
  point[1] = rwi->GetEventPosition()[1];
  point[1] = rwi->GetSize()[1] - point[1]; // flip y axis

  int button = 0;
  int modifiers = 0;
  int buttonState = 0;

  // vtkRenderWindowInteractor does not provide information about mouse buttons during
  // mouse move events therefore the Press/Release functions update the button state in the buttonStateMap
  // this button state is then provided for mouse move events

  if (buttonStateMap.find(sender) != buttonStateMap.end())
    buttonState = buttonStateMap.find(sender)->second;

  if (buttonState != 0 && vtkCommandEventId != vtkCommand::MouseMoveEvent)
    buttonStateMap.erase(buttonStateMap.find(sender)); //press or release event with active map caching

  switch (vtkCommandEventId)
  {
  case vtkCommand::LeftButtonReleaseEvent:
    button = mitk::LeftMouseButton;
    modifiers = buttonState;
    // remove left mouse button from button state
    buttonStateMap[sender] = (buttonState - button);
    break;
  case vtkCommand::MiddleButtonReleaseEvent:
    button = mitk::MiddleMouseButton;
    modifiers = buttonState;
    // remove middle button from button state
    buttonStateMap[sender] = (buttonState - button);
    break;
  case vtkCommand::RightButtonReleaseEvent:
    button = mitk::RightMouseButton;
    modifiers = buttonState;
    // remove right mouse button from button state
    buttonStateMap[sender] = (buttonState - button);
    break;
  }

  if (rwi->GetShiftKey())
  {
    modifiers |= mitk::ShiftKey;
  }
  if (rwi->GetControlKey())
  {
    modifiers |= mitk::ControlKey;
  }
  if (rwi->GetAltKey())
  {
    modifiers |= mitk::AltKey;
  }

  MouseReleaseEvent::Pointer mre = MouseReleaseEvent::New(sender, point, (MouseButtons) buttonState, (ModifierKeys) modifiers,
      (MouseButtons) button);
  return mre;
}

mitk::MouseWheelEvent::Pointer mitk::VtkEventAdapter::AdaptMouseWheelEvent(mitk::BaseRenderer* sender,
    unsigned long vtkCommandEventId,
    vtkRenderWindowInteractor* rwi)
{
  mitk::Point2D p;
  p[0] = rwi->GetEventPosition()[0];
  p[1] = rwi->GetEventPosition()[1];

  int delta = 0;
  switch (vtkCommandEventId)
  {
  case vtkCommand::MouseWheelForwardEvent:
    delta = +120;         //http://doc.trolltech.com/3.3/qwheelevent.html#delta
    break;
  case vtkCommand::MouseWheelBackwardEvent:
    delta = -120;         //http://doc.trolltech.com/3.3/qwheelevent.html#delta
    break;
  }

  int modifiers = 0;
  if (rwi->GetShiftKey())
  {
    modifiers |= mitk::BS_ShiftButton;}
  if (rwi->GetControlKey())
  {
    modifiers |= mitk::BS_ControlButton;}
  if (rwi->GetAltKey())
  {
    modifiers |= mitk::BS_AltButton;
  }

  mitk::Point2D point;
  point[0] = rwi->GetEventPosition()[0];
  point[1] = rwi->GetEventPosition()[1];
  point[1] = rwi->GetSize()[1] - point[1]; // flip y axis

  // vtkWheelEvent does not have a buttonState or event button
  int buttonState = 0;
  MouseWheelEvent::Pointer mpe = MouseWheelEvent::New(sender, point, (MouseButtons) buttonState, (ModifierKeys) modifiers, delta);
  return mpe;
}

mitk::InteractionKeyEvent::Pointer mitk::VtkEventAdapter::AdaptInteractionKeyEvent(mitk::BaseRenderer* sender,
    unsigned long vtkCommandEventId,
    vtkRenderWindowInteractor* rwi)
{
  if (vtkCommandEventId != vtkCommand::KeyPressEvent)
  {
    MITK_WARN<< "mitk::VtkEventAdapter::AdaptInteractionKeyEvent() called with wrong argument";
    return NULL;
  }
  int modifiers = 0;
  if (rwi->GetShiftKey())
  {
    modifiers |= mitk::BS_ShiftButton;}
  if (rwi->GetControlKey())
  {
    modifiers |= mitk::BS_ControlButton;}
  if (rwi->GetAltKey())
  {
    modifiers |= mitk::BS_AltButton;
  }
  InteractionKeyEvent::Pointer ike = InteractionKeyEvent::New(sender, std::string(rwi->GetKeySym()), (ModifierKeys)modifiers );
  return ike;

}
