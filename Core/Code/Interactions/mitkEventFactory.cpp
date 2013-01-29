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
  std::vector<std::string> elems;
  return split(s, delim, elems);
}

mitk::InteractionEvent::Pointer mitk::EventFactory::CreateEvent(PropertyList::Pointer list)
{
  //
  std::string eventClass, eventVariant;
  list->GetStringProperty(EVENTCLASS.c_str(), eventClass);
  list->GetStringProperty(EVENTVARIANT.c_str(), eventVariant);

  // Query all possible attributes, if they are not present, set their default values.
  // Position Events & Key Events
  std::string strModifiers;
  ModifierKeys modifiers = NoKey;
  std::string strEventButton;
  MouseButtons eventButton = NoButton;
  std::string strButtonState;
  MouseButtons buttonState = NoButton;
  std::string strKey;
  std::string key;
  std::string strWheelDelta;
  int wheelDelta;
  std::string strSignalName = "";

  Point2D pos;

  // Parse modifier information
  if (list->GetStringProperty(MODIFIERS.c_str(), strModifiers))
  {
    std::vector<std::string> mods = split(strModifiers, ',');
    for (std::vector<std::string>::iterator it = mods.begin(); it != mods.end(); ++it)
    {
      std::transform((*it).begin(), (*it).end(), (*it).begin(), ::toupper);
      if (*it == "CTRL")
      {
        modifiers = modifiers | ControlKey;
      }
      else if (*it == "ALT")
      {
        modifiers = modifiers | AltKey;
      }
      else if (*it == "SHIFT")
      {
        modifiers = modifiers | ShiftKey;
      }
      else
      {
        MITK_WARN<< "mitkEventFactory: Invalid event modifier in config file :" << (*it);
      }
    }
  }

  // Set EventButton
  if (list->GetStringProperty(EVENTBUTTON.c_str(), strEventButton))
  {
    std::transform(strEventButton.begin(), strEventButton.end(), strEventButton.begin(), ::toupper);
    if (strEventButton == "MIDDLEMOUSEBUTTON")
    {
      eventButton = MiddleMouseButton;
    }
    else if (strEventButton == "LEFTMOUSEBUTTON")
    {
      eventButton = LeftMouseButton;
    }
    else if (strEventButton == "RIGHTMOUSEBUTTON")
    {
      eventButton = RightMouseButton;
    }
    else
    {
      MITK_WARN<< "mitkEventFactory: Invalid event button in config file: " << strEventButton;
    }
  }

  // Parse ButtonStates
  if (list->GetStringProperty(BUTTONSTATE.c_str(), strButtonState))
  {
    std::vector<std::string> mods = split(strButtonState, ',');
    for (std::vector<std::string>::iterator it = mods.begin(); it != mods.end(); ++it)
    {
      std::transform((*it).begin(), (*it).end(), (*it).begin(), ::toupper);
      if (*it == "MIDDLEMOUSEBUTTON")
      {
        buttonState = buttonState | MiddleMouseButton;
      }
      else if (*it == "LEFTMOUSEBUTTON")
      {
        buttonState = buttonState | LeftMouseButton;
      }
      else if (*it == "RIGHTMOUSEBUTTON")
      {
        buttonState = buttonState | RightMouseButton;
      }
      else
      {
        MITK_WARN<< "mitkEventFactory: Invalid event buttonstate in config file:" << (*it);
      }
    }
  }

  // Key
  if (!list->GetStringProperty(KEY.c_str(), strKey))
  {
    key = "";
  }
  else
  {
    key = strKey;
  }
  // WheelDelta
  if (!list->GetStringProperty(WHEELDELTA.c_str(), strWheelDelta))
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
  list->GetStringProperty(SIGNALNAME.c_str(), strSignalName);

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
  else if (eventClass == "KEYEVENT")
  {
    event = InteractionKeyEvent::New(NULL, key, modifiers);
  }
  else if (eventClass == "MOUSEWHEELEVENT")
  {
    event = MouseWheelEvent::New(NULL, pos, buttonState, modifiers, wheelDelta);
  }
  else if (eventClass == "POSITIONEVENT")
  {
    event = InteractionPositionEvent::New(NULL, pos, "PositionEvent");
  }
  else if (eventClass == "INTERNALEVENT")
  {
    event = InternalEvent::New(NULL, NULL, strSignalName);
  }
  if (event.IsNull())
  {
    MITK_WARN<< "Event couldn't be constructed. Please check your StateMachine patterns and config files\n for the following event class, which is not valid: " << eventClass;
  }
  return event;
}

