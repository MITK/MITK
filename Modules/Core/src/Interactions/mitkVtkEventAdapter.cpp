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
#include "mitkInteractionEventConst.h"
#include <string.h>
#include "vtkCommand.h"

namespace mitk
{
std::map<BaseRenderer*, int> VtkEventAdapter::buttonStateMap;
}

mitk::MouseMoveEvent::Pointer mitk::VtkEventAdapter::AdaptMouseMoveEvent(mitk::BaseRenderer* sender,
                                                                         unsigned long vtkCommandEventId,
                                                                         vtkRenderWindowInteractor* rwi)
{
  mitk::Point2D point;
  point[0] = rwi->GetEventPosition()[0];
  point[1] = rwi->GetEventPosition()[1];

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
    modifiers |= InteractionEvent::ShiftKey;
  }
  if (rwi->GetControlKey())
  {
    modifiers |= InteractionEvent::ControlKey;
  }
  if (rwi->GetAltKey())
  {
    modifiers |= InteractionEvent::AltKey;
  }

  MouseMoveEvent::Pointer mme = MouseMoveEvent::New(sender, point,
                                                    static_cast<InteractionEvent::MouseButtons>(buttonState),
                                                    static_cast<InteractionEvent::ModifierKeys>(modifiers));
  return mme;
}

mitk::MouseReleaseEvent::Pointer mitk::VtkEventAdapter::AdaptMouseReleaseEvent(mitk::BaseRenderer* sender,
                                                                               unsigned long vtkCommandEventId,
                                                                               vtkRenderWindowInteractor* rwi)
{
  mitk::Point2D point;
  point[0] = rwi->GetEventPosition()[0];
  point[1] = rwi->GetEventPosition()[1];

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
    button = InteractionEvent::LeftMouseButton;
    // remove left mouse button from button state
    buttonStateMap[sender] = (buttonState - button);
    break;
  case vtkCommand::MiddleButtonReleaseEvent:
    button = InteractionEvent::MiddleMouseButton;
    // remove middle button from button state
    buttonStateMap[sender] = (buttonState - button);
    break;
  case vtkCommand::RightButtonReleaseEvent:
    button = InteractionEvent::RightMouseButton;
    // remove right mouse button from button state
    buttonStateMap[sender] = (buttonState - button);
    break;
  }

  if (rwi->GetShiftKey())
  {
    modifiers |= InteractionEvent::ShiftKey;
  }
  if (rwi->GetControlKey())
  {
    modifiers |= InteractionEvent::ControlKey;
  }
  if (rwi->GetAltKey())
  {
    modifiers |= InteractionEvent::AltKey;
  }

  // after releasing button is no longer pressed, to update it
  if (buttonStateMap.find(sender) != buttonStateMap.end()) {
    buttonState = buttonStateMap.find(sender)->second;
  }

  MouseReleaseEvent::Pointer mre = MouseReleaseEvent::New(sender, point,
                                                          static_cast<InteractionEvent::MouseButtons>(buttonState),
                                                          static_cast<InteractionEvent::ModifierKeys>(modifiers),
                                                          static_cast<InteractionEvent::MouseButtons>(button));
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
    modifiers |= mitk::BS_ShiftButton;
  }
  if (rwi->GetControlKey())
  {
    modifiers |= mitk::BS_ControlButton;
  }
  if (rwi->GetAltKey())
  {
    modifiers |= mitk::BS_AltButton;
  }

  mitk::Point2D point;
  point[0] = rwi->GetEventPosition()[0];
  point[1] = rwi->GetEventPosition()[1];

  // vtkWheelEvent does not have a buttonState or event button
  int buttonState = 0;
  MouseWheelEvent::Pointer mpe = MouseWheelEvent::New(sender, point,
                                                      static_cast<InteractionEvent::MouseButtons>(buttonState),
                                                      static_cast<InteractionEvent::ModifierKeys>(modifiers),
                                                      delta);
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
  InteractionKeyEvent::Pointer ike = InteractionKeyEvent::New(sender, std::string(rwi->GetKeySym()),
                                                              static_cast<InteractionEvent::ModifierKeys>(modifiers));
  return ike;

}


mitk::MousePressEvent::Pointer mitk::VtkEventAdapter::AdaptMousePressEvent(mitk::BaseRenderer* sender,
                                                                           unsigned long vtkCommandEventId,
                                                                           vtkRenderWindowInteractor* rwi)
{
  mitk::Point2D point;
  point[0] = rwi->GetEventPosition()[0];
  point[1] = rwi->GetEventPosition()[1];


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
    button = InteractionEvent::LeftMouseButton;
    buttonState |= button;
    buttonStateMap[sender] = buttonState;
    break;
  case vtkCommand::MiddleButtonPressEvent:
    button = InteractionEvent::MiddleMouseButton;
    buttonState |= button;
    buttonStateMap[sender] = buttonState;
    break;
  case vtkCommand::RightButtonPressEvent:
    button = InteractionEvent::RightMouseButton;
    buttonState |= button;
    buttonStateMap[sender] = buttonState;
    break;
  }

  if (rwi->GetShiftKey())
  {
    modifiers |= InteractionEvent::ShiftKey;
  }
  if (rwi->GetControlKey())
  {
    modifiers |= InteractionEvent::ControlKey;
  }
  if (rwi->GetAltKey())
  {
    modifiers |= InteractionEvent::AltKey;
  }

  MousePressEvent::Pointer mpe = MousePressEvent::New(sender, point,
                                                      static_cast<InteractionEvent::MouseButtons>(buttonState),
                                                      static_cast<InteractionEvent::ModifierKeys>(modifiers),
                                                      static_cast<InteractionEvent::MouseButtons>(button));
  return mpe;
}
