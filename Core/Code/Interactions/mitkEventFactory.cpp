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

#include "mitkEventFactory.h"
#include <algorithm>
#include <sstream>
#include <string>
#include <mitkMousePressEvent.h>
#include <mitkMouseMoveEvent.h>
#include <mitkMouseReleaseEvent.h>
#include <mitkMouseWheelEvent.h>
#include <mitkInteractionKeyEvent.h>
#include <mitkInteractionEventConst.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkInternalEvent.h>

namespace
{
  std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems)
  {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
      elems.push_back(item);
    }
    return elems;
  }

  std::vector<std::string> split(const std::string &s, char delim)
  {
    std::vector < std::string > elems;
    return split(s, delim, elems);
  }
}

mitk::InteractionEvent::Pointer mitk::EventFactory::CreateEvent(PropertyList::Pointer list)
{
//
  std::string eventClass, eventVariant;
  list->GetStringProperty(InteractionEventConst::xmlParameterEventClass().c_str(), eventClass);
  list->GetStringProperty(InteractionEventConst::xmlParameterEventVariant().c_str(), eventVariant);

// Query all possible attributes, if they are not present, set their default values.
// Position Events & Key Events
  std::string strModifiers;
  InteractionEvent::ModifierKeys modifiers = InteractionEvent::NoKey;
  std::string strEventButton;
  InteractionEvent::MouseButtons eventButton = InteractionEvent::NoButton;
  std::string strButtonState;
  InteractionEvent::MouseButtons buttonState = InteractionEvent::NoButton;
  std::string strKey;
  std::string key;
  std::string strWheelDelta;
  int wheelDelta;
  std::string strSignalName = "";

  Point2D pos;
  pos.Fill(0);

// Parse modifier information
  if (list->GetStringProperty(InteractionEventConst::xmlEventPropertyModifier().c_str(), strModifiers))
  {
    std::vector<std::string> mods = split(strModifiers, ',');
    for (std::vector<std::string>::iterator it = mods.begin(); it != mods.end(); ++it)
    {
      std::transform((*it).begin(), (*it).end(), (*it).begin(), ::toupper);
      if (*it == "CTRL")
      {
        modifiers = modifiers | InteractionEvent::ControlKey;
      }
      else if (*it == "ALT")
      {
        modifiers = modifiers | InteractionEvent::AltKey;
      }
      else if (*it == "SHIFT")
      {
        modifiers = modifiers | InteractionEvent::ShiftKey;
      }
      else
      {
        MITK_WARN<< "mitkEventFactory: Invalid event modifier in config file :" << (*it);
      }
    }
  }

// Set EventButton
  if (list->GetStringProperty(InteractionEventConst::xmlEventPropertyEventButton().c_str(), strEventButton))
  {
    std::transform(strEventButton.begin(), strEventButton.end(), strEventButton.begin(), ::toupper);
    if (strEventButton == "MIDDLEMOUSEBUTTON")
    {
      eventButton = InteractionEvent::MiddleMouseButton;
    }
    else if (strEventButton == "LEFTMOUSEBUTTON")
    {
      eventButton = InteractionEvent::LeftMouseButton;
    }
    else if (strEventButton == "RIGHTMOUSEBUTTON")
    {
      eventButton = InteractionEvent::RightMouseButton;
    }
    else
    {
      MITK_WARN<< "mitkEventFactory: Invalid event button in config file: " << strEventButton;
    }
  }

// Parse ButtonStates
  if (list->GetStringProperty(InteractionEventConst::xmlEventPropertyButtonState().c_str(), strButtonState))
  {
    std::vector<std::string> mods = split(strButtonState, ',');
    for (std::vector<std::string>::iterator it = mods.begin(); it != mods.end(); ++it)
    {
      std::transform((*it).begin(), (*it).end(), (*it).begin(), ::toupper);
      if (*it == "MIDDLEMOUSEBUTTON")
      {
        buttonState = buttonState | InteractionEvent::MiddleMouseButton;
      }
      else if (*it == "LEFTMOUSEBUTTON")
      {
        buttonState = buttonState | InteractionEvent::LeftMouseButton;
      }
      else if (*it == "RIGHTMOUSEBUTTON")
      {
        buttonState = buttonState | InteractionEvent::RightMouseButton;
      }
      else
      {
        MITK_WARN<< "mitkEventFactory: Invalid event buttonstate in config file:" << (*it);
      }
    }
  }

// Key
  if (!list->GetStringProperty(InteractionEventConst::xmlEventPropertyKey().c_str(), strKey))
  {
    key = "";
  }
  else
  {
    key = strKey;
  }
// WheelDelta
  if (!list->GetStringProperty(InteractionEventConst::xmlEventPropertyScrollDirection().c_str(), strWheelDelta))
  {
    wheelDelta = 0;
  }
  else
  {
    std::transform(strWheelDelta.begin(), strWheelDelta.end(), strWheelDelta.begin(), ::toupper);
    if (strWheelDelta == "DOWN")
    {
      wheelDelta = -1;
    }
    else
    {
      wheelDelta = 1;
    }
  }
// Internal Signals Name
  list->GetStringProperty(InteractionEventConst::xmlEventPropertySignalName().c_str(), strSignalName);

  /*
   * Here the objects are created
   */

  mitk::InteractionEvent::Pointer event;
  std::transform(eventClass.begin(), eventClass.end(), eventClass.begin(), ::toupper);

  if (eventClass == "MOUSEPRESSEVENT")
  {
    // buttonstates incorporate the event button (as in Qt)
    buttonState = buttonState | eventButton;
    event = MousePressEvent::New(NULL, pos, buttonState, modifiers, eventButton);
  }
  else if (eventClass == "MOUSEMOVEEVENT")
  {
    event = MouseMoveEvent::New(NULL, pos, buttonState, modifiers);
  }
  else if (eventClass == "MOUSERELEASEEVENT")
  {
    event = MouseReleaseEvent::New(NULL, pos, buttonState, modifiers, eventButton);
  }
  else if (eventClass == "INTERACTIONKEYEVENT")
  {
    event = InteractionKeyEvent::New(NULL, key, modifiers);
  }
  else if (eventClass == "MOUSEWHEELEVENT")
  {
    event = MouseWheelEvent::New(NULL, pos, buttonState, modifiers, wheelDelta);
  }
  else if (eventClass == "INTERACTIONPOSITIONEVENT")
  {
    event = InteractionPositionEvent::New(NULL, pos);
  }
  else if (eventClass == "INTERNALEVENT")
  {
    event = InternalEvent::New(NULL, NULL, strSignalName);
  }
  else if (eventClass == "INTERACTIONEVENT")
  {
    event = InteractionEvent::New(NULL);
  }
  if (event.IsNull())
  {
    MITK_WARN<< "Event couldn't be constructed. Please check your StateMachine patterns and config files\n for the following event class, which is not valid: " << eventClass;
    return NULL;
  }
  return event;
}
