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
      if (*it == "ctrl")
      {
        modifiers = modifiers | ControlKey;
      }
      else if (*it == "alt")
      {
        modifiers = modifiers | AltKey;
      }
      else if (*it == "shift")
      {
        modifiers = modifiers | ShiftKey;
      }
      else
      {
        MITK_WARN<< "mitkEventFactory: Invalid event modifier in config file:" << modifiers;
      }
    }
  }

  // Set EventButton
  if (list->GetStringProperty(EVENTBUTTON.c_str(), strEventButton))
  {
    if (strEventButton == "MiddleMouseButton")
    {
      eventButton = MiddleMouseButton;
    }
    else if (strEventButton == "LeftMouseButton")
    {
      eventButton = LeftMouseButton;
    }
    else if (strEventButton == "RightMouseButton")
    {
      eventButton = RightMouseButton;
    }
    else
    {
      MITK_WARN<< "mitkEventFactory: Invalid event button in config file:" << eventButton;
    }
  }

  // Parse ButtonStates
  if (list->GetStringProperty(BUTTONSTATE.c_str(), strButtonState))
  {
    std::vector<std::string> mods = split(strButtonState, ',');
    for (std::vector<std::string>::iterator it = mods.begin(); it != mods.end(); ++it)
    {
      if (*it == "MiddleMouseButton")
      {
        buttonState = buttonState | MiddleMouseButton;
      }
      else if (*it == "LeftMouseButton")
      {
        buttonState = buttonState | LeftMouseButton;
      }
      else if (*it == "RightMouseButton")
      {
        buttonState = buttonState | RightMouseButton;
      }
      else
      {
        MITK_WARN<< "mitkEventFactory: Invalid event buttonstate in config file:" << eventButton;
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
    wheelDelta = atoi(strWheelDelta.c_str());
  }
  // Internal Signals Name
  list->GetStringProperty(SIGNALNAME.c_str(), strSignalName);

  /*
   * Here the objects are created
   */

  mitk::InteractionEvent::Pointer event;

  if (eventClass == "MousePressEvent")
  {
    // buttonstates incorporate the event button (as in Qt)
    buttonState = buttonState | eventButton;
    event = MousePressEvent::New(NULL, pos, buttonState, modifiers, eventButton);
  }
  else if (eventClass == "MouseMoveEvent")
  {
    event = MouseMoveEvent::New(NULL, pos, buttonState, modifiers);
  }
  else if (eventClass == "MouseReleaseEvent")
  {
    event = MouseReleaseEvent::New(NULL, pos, buttonState, modifiers, eventButton);
  }
  else if (eventClass == "KeyEvent")
  {
    event = InteractionKeyEvent::New(NULL, key, modifiers);
  }
  else if (eventClass == "MouseWheelEvent")
  {
    event = MouseWheelEvent::New(NULL, pos, buttonState, modifiers, wheelDelta);
  }
  else if (eventClass == "PositionEvent")
  {
    event = InteractionPositionEvent::New(NULL, pos, "PositionEvent");
    MITK_INFO << event;
  }
  else if (eventClass == "InternalEvent")
  {
    event = InternalEvent::New(NULL, NULL, strSignalName);
  }
  if (event.IsNull()) {
    MITK_WARN << "Event couldn't be constructed. Please check your StateMachine patterns and config files\n for the following event class, which is not valid: " << eventClass;
  }
  return event;
}

