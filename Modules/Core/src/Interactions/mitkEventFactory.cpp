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
#include <mitkMouseDoubleClickEvent.h>
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


/**
 * @brief GetEventButton Return EventButton as String
 * @param event
 * @return
 */
static std::string GetButtonState(mitk::InteractionEvent *event)
{
  mitk::InteractionEvent::MouseButtons buttonState = mitk::InteractionEvent::NoButton;
  std::string eventClass = event->GetNameOfClass();
  std::transform(eventClass.cbegin(), eventClass.cend(), eventClass.begin(), ::toupper);

  std::string strButtonState = "";
  if (eventClass == "MOUSEPRESSEVENT")
  {
    mitk::MousePressEvent* mme = dynamic_cast<mitk::MousePressEvent*> (event);
    buttonState = mme->GetButtonStates();
  }
  if (eventClass == "MOUSERELEASEEVENT")
  {
    mitk::MouseReleaseEvent* mme = dynamic_cast<mitk::MouseReleaseEvent*> (event);
    buttonState = mme->GetButtonStates();
  }
  if (eventClass == "MOUSEDOUBLECLICKEVENT")
  {
    mitk::MouseDoubleClickEvent* mme = dynamic_cast<mitk::MouseDoubleClickEvent*> (event);
    buttonState = mme->GetButtonStates();
  }
  if (eventClass == "MOUSEMOVEEVENT")
  {
    mitk::MouseMoveEvent* mme = dynamic_cast<mitk::MouseMoveEvent*> (event);
    buttonState = mme->GetButtonStates();
  }
  if (eventClass == "MOUSEWHEELEVENT")
  {
    mitk::MouseWheelEvent* mme = dynamic_cast<mitk::MouseWheelEvent*> (event);
    buttonState = mme->GetButtonStates();
  }

  if (buttonState & mitk::InteractionEvent::LeftMouseButton )
  {
    strButtonState = "LeftMouseButton";
  }
  if (buttonState & mitk::InteractionEvent::RightMouseButton )
  {
    if (strButtonState != "")
      strButtonState += ",";

    strButtonState += "RightMouseButton";
  }
  if (buttonState & mitk::InteractionEvent::MiddleMouseButton )
  {
    if (strButtonState != "")
      strButtonState += ",";

    strButtonState += "MiddleMouseButton";
  }
  return strButtonState;
}

/**
 * @brief GetModifierState Return ModifierState as String
 * @param event
 * @return
 */
static std::string GetModifierState(mitk::InteractionEvent *event)
{
  mitk::InteractionEvent::ModifierKeys modifierKeys = mitk::InteractionEvent::NoKey;
  std::string eventClass = event->GetNameOfClass();
  std::transform(eventClass.cbegin(), eventClass.cend(), eventClass.begin(), ::toupper);
  std::string strModKeys = "";
  // TODO Add InteractionKey
  if (eventClass == "MOUSEPRESSEVENT")
  {
    mitk::MousePressEvent* mme = dynamic_cast<mitk::MousePressEvent*> (event);
    modifierKeys = mme->GetModifiers();
  }
  if (eventClass == "MOUSERELEASEEVENT")
  {
    mitk::MouseReleaseEvent* mme = dynamic_cast<mitk::MouseReleaseEvent*> (event);
    modifierKeys = mme->GetModifiers();
  }
  if (eventClass == "MOUSEDOUBLECLICKEVENT")
  {
    mitk::MouseDoubleClickEvent* mme = dynamic_cast<mitk::MouseDoubleClickEvent*> (event);
    modifierKeys = mme->GetModifiers();
  }
  if (eventClass == "MOUSEMOVEEVENT")
  {
    mitk::MouseMoveEvent* mme = dynamic_cast<mitk::MouseMoveEvent*> (event);
    modifierKeys = mme->GetModifiers();
  }
  if (eventClass == "MOUSEWHEELEVENT")
  {
    mitk::MouseWheelEvent* mme = dynamic_cast<mitk::MouseWheelEvent*> (event);
    modifierKeys = mme->GetModifiers();
  }

  if (modifierKeys & mitk::InteractionEvent::ShiftKey )
  {
    strModKeys = "SHIFT";
  }
  if (modifierKeys & mitk::InteractionEvent::ControlKey )
  {
    if (strModKeys != "")
      strModKeys += ",";

    strModKeys += "CTRL";
  }
  if (modifierKeys & mitk::InteractionEvent::AltKey )
  {
    if (strModKeys != "")
      strModKeys += ",";

    strModKeys += "ALT";
  }
  return strModKeys;
}

/**
 * @brief GetEventButton Return EventButton as String
 * @param event
 * @return
 */
static std::string GetEventButton(mitk::InteractionEvent *event)
{
  mitk::InteractionEvent::MouseButtons button = mitk::InteractionEvent::NoButton;
  std::string eventClass = event->GetNameOfClass();
  std::transform(eventClass.cbegin(), eventClass.cend(), eventClass.begin(), ::toupper);
  std::string stdButton = "";
  // TODO Add InteractionKey
  if (eventClass == "MOUSEPRESSEVENT")
  {
    mitk::MousePressEvent* mme = dynamic_cast<mitk::MousePressEvent*> (event);
    button = mme->GetEventButton();
  }
  if (eventClass == "MOUSERELEASEEVENT")
  {
    mitk::MouseReleaseEvent* mme = dynamic_cast<mitk::MouseReleaseEvent*> (event);
    button = mme->GetEventButton();
  }
  if (eventClass == "MOUSEDOUBLECLICKEVENT")
  {
    mitk::MouseDoubleClickEvent* mme = dynamic_cast<mitk::MouseDoubleClickEvent*> (event);
    button = mme->GetEventButton();
  }

  if (button & mitk::InteractionEvent::LeftMouseButton )
  {
    stdButton = "LeftMouseButton";
  }
  if (button & mitk::InteractionEvent::RightMouseButton )
  {
    stdButton = "RightMouseButton";
  }
  if (button & mitk::InteractionEvent::MiddleMouseButton )
  {
    stdButton = "MiddleMouseButton";
  }
  return stdButton;

}

/**
 * @brief GetPosition Return World Position as String
 * @param event
 * @return
 */
static std::string GetPositionInWorld(mitk::InteractionEvent *event)
{
  std::stringstream ss;
  mitk::InteractionPositionEvent* pe = dynamic_cast<mitk::InteractionPositionEvent*> (event);
  if (pe != NULL)
  {
    mitk::Point3D p = pe->GetPositionInWorld();
    ss << p[0] << "," << p[1] << "," << p[2];
  }
  return ss.str();
}

/**
 * @brief GetPositionOnScreen Return PositionOnScreen as String
 * @param event
 * @return
 */
static std::string GetPositionOnScreen(mitk::InteractionEvent *event)
{
  std::stringstream ss;
  mitk::InteractionPositionEvent* pe = dynamic_cast<mitk::InteractionPositionEvent*> (event);
  if (pe != NULL)
  {
    mitk::Point2D p = pe->GetPointerPositionOnScreen();
    ss << p[0] << "," << p[1];
  }
  return ss.str();
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
  std::string strPos;

  // Position on screen
  if( list->GetStringProperty(InteractionEventConst::xmlEventPropertyPositionOnScreen().c_str(), strPos))
  {
    //split comma separated string
    int commaPos;
    commaPos = strPos.find_first_of(',');

    pos[0] = static_cast<mitk::ScalarType>(std::atof(strPos.substr(0, commaPos).c_str()));
    pos[1] = static_cast<mitk::ScalarType>(std::atof(strPos.substr(commaPos+1, strPos.length()).c_str()));
  }

  std::string strWorld;
  Point3D worldPos;
  worldPos.Fill(0);
  //Position in world coordinates
  if(list->GetStringProperty(InteractionEventConst::xmlEventPropertyPositionInWorld().c_str(), strWorld))
  {
    const std::vector<std::string> coords = split(strWorld, ',');
    int i = 0;
    for ( std::vector<std::string>::const_iterator it = coords.cbegin(); it != coords.cend(); ++it, ++i)
    {
      worldPos[i] = atof((*it).c_str());
    }
  }

  // Parse modifier information
  if (list->GetStringProperty(InteractionEventConst::xmlEventPropertyModifier().c_str(), strModifiers))
  {
    std::vector<std::string> mods = split(strModifiers, ',');
    for (std::vector<std::string>::iterator it = mods.begin(); it != mods.end(); ++it)
    {
      std::transform((*it).cbegin(), (*it).cend(), (*it).begin(), ::toupper);
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
    std::transform(strEventButton.cbegin(), strEventButton.cend(), strEventButton.begin(), ::toupper);
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
      std::transform((*it).cbegin(), (*it).cend(), (*it).begin(), ::toupper);
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
    std::transform(strWheelDelta.cbegin(), strWheelDelta.cend(), strWheelDelta.begin(), ::toupper);
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

  // Get BaseRenderer by name
  mitk::BaseRenderer* renderer = NULL;
  std::string strRenderer;

  // only search for a renderer if there is at least one renderer registered
  if(mitk::BaseRenderer::baseRendererMap.size() > 0)
  {

    if(list->GetStringProperty(mitk::InteractionEventConst::xmlEventPropertyRendererName().c_str(), strRenderer))
    {
      //look up for renderer registered with the name in xml file
      renderer = mitk::BaseRenderer::GetByName(strRenderer);
    }

    //if not found always use first registered renderer
    if(renderer == NULL)
      renderer = (*(mitk::BaseRenderer::baseRendererMap.cbegin())).second;
}

  /*
   * Here the objects are created
   */

  mitk::InteractionEvent::Pointer event;
  std::transform(eventClass.cbegin(), eventClass.cend(), eventClass.begin(), ::toupper);

  if (eventClass == "MOUSEPRESSEVENT")
  {
    // buttonstates incorporate the event button (as in Qt)
    buttonState = buttonState | eventButton;
    event = MousePressEvent::New(renderer, pos, buttonState, modifiers, eventButton);
  }
  else if (eventClass == "MOUSEDOUBLECLICKEVENT")
  {
    buttonState = buttonState | eventButton;
    event = MouseDoubleClickEvent::New(renderer, pos, buttonState, modifiers, eventButton);
  }
  else if (eventClass == "MOUSEMOVEEVENT")
  {
    event = MouseMoveEvent::New(renderer, pos, buttonState, modifiers);
  }
  else if (eventClass == "MOUSERELEASEEVENT")
  {
    event = MouseReleaseEvent::New(renderer, pos, buttonState, modifiers, eventButton);
  }
  else if (eventClass == "INTERACTIONKEYEVENT")
  {
    event = InteractionKeyEvent::New(renderer, key, modifiers);
  }
  else if (eventClass == "MOUSEWHEELEVENT")
  {
    event = MouseWheelEvent::New(renderer, pos, buttonState, modifiers, wheelDelta);
  }
  else if (eventClass == "INTERACTIONPOSITIONEVENT")
  {
    event = InteractionPositionEvent::New(renderer, pos);
  }
  else if (eventClass == "INTERNALEVENT")
  {
    event = InternalEvent::New(renderer, NULL, strSignalName);
  }
  else if (eventClass == "INTERACTIONEVENT")
  {
    event = InteractionEvent::New(renderer);
  }
  if (event.IsNull())
  {
    MITK_WARN<< "Event couldn't be constructed. Please check your StateMachine patterns and config files\n for the following event class, which is not valid: " << eventClass;
    return NULL;
  }
  return event;
}

std::string mitk::EventFactory::EventToXML(mitk::InteractionEvent *event)
{

  InternalEvent* ie = dynamic_cast<InternalEvent*> (event);
  if (ie != NULL)
    return "";

  std::string eventClass = event->GetNameOfClass();
  std::string eventXML = "<" + InteractionEventConst::xmlTagEventVariant() +  " " + InteractionEventConst::xmlParameterEventClass() + "=\"";

  std::transform(eventClass.cbegin(), eventClass.cend(), eventClass.begin(), ::toupper);

  eventXML += eventClass + "\" >\n";
  // here follow event specific attributes
  if (eventClass == "MOUSEPRESSEVENT" || eventClass == "MOUSERELEASEEVENT" || eventClass == "MOUSEDOUBLECLICKEVENT" || eventClass == "MOUSEMOVEEVENT" || eventClass == "MOUSEWHEELEVENT")
  {
    if (!(eventClass == "MOUSEMOVEEVENT") && !(eventClass == "MOUSEWHEELEVENT"))
    {
      // EventButton
      eventXML += " <" + InteractionEventConst::xmlTagAttribute() +" " + InteractionEventConst::xmlParameterName() + "=\"" + InteractionEventConst::xmlEventPropertyEventButton() + "\" ";
      eventXML += InteractionEventConst::xmlParameterValue() + "=\"";
      eventXML += GetEventButton(event);
      eventXML += "\"/>\n";
    }
    // ButtonState
    if (GetButtonState(event) != "")
    {
      eventXML += " <" + InteractionEventConst::xmlTagAttribute() +" " + InteractionEventConst::xmlParameterName() + "=\"" + InteractionEventConst::xmlEventPropertyButtonState() + "\" ";
      eventXML += InteractionEventConst::xmlParameterValue() + "=\"";
      eventXML += GetButtonState(event);
      eventXML += "\"/>\n";
    }

    // Modifiers
    if (GetModifierState(event) != "")
    {
      eventXML += " <" + InteractionEventConst::xmlTagAttribute() +" " + InteractionEventConst::xmlParameterName() + "=\"" + InteractionEventConst::xmlEventPropertyModifier() + "\" ";
      eventXML += InteractionEventConst::xmlParameterValue() + "=\"";
      eventXML += GetModifierState(event);
      eventXML += "\"/>\n";
    }

    // Position on Screen
    eventXML += " <" + InteractionEventConst::xmlTagAttribute() +" " + InteractionEventConst::xmlParameterName() + "=\"" + InteractionEventConst::xmlEventPropertyPositionOnScreen() + "\" ";
    eventXML += InteractionEventConst::xmlParameterValue() + "=\"";
    eventXML += GetPositionOnScreen(event);
    eventXML += "\"/>\n";

    // Position in World
    eventXML += " <" + InteractionEventConst::xmlTagAttribute() +" " + InteractionEventConst::xmlParameterName() + "=\"" + InteractionEventConst::xmlEventPropertyPositionInWorld() + "\" ";
    eventXML += InteractionEventConst::xmlParameterValue() + "=\"";
    eventXML += GetPositionInWorld(event);
    eventXML += "\"/>\n";
  }
  else if (eventClass == "INTERACTIONKEYEVENT")
  {
    mitk::InteractionKeyEvent* ke = dynamic_cast<mitk::InteractionKeyEvent*>(event);

    // key
    eventXML += " <" + InteractionEventConst::xmlTagAttribute() +" " + InteractionEventConst::xmlParameterName() + "=\"" + InteractionEventConst::xmlEventPropertyKey() + "\" ";
    eventXML += InteractionEventConst::xmlParameterValue() + "=\"";
    eventXML += ke->GetKey();
    eventXML += "\"/>\n";
  }
  else
  {
    MITK_WARN << "Event not recognized, discarding event of type " << event->GetNameOfClass();
  }
  if (eventClass == "MOUSEWHEELEVENT")
  {
    MouseWheelEvent* we = dynamic_cast<MouseWheelEvent*> (event);
    int delta = we->GetWheelDelta();

    std::stringstream ss;
    ss << delta;

    eventXML += " <" + InteractionEventConst::xmlTagAttribute() +" " + InteractionEventConst::xmlParameterName() + "=\"" + InteractionEventConst::xmlEventPropertyWheelDelta() + "\" ";
    eventXML += InteractionEventConst::xmlParameterValue() + "=\"";
    eventXML += ss.str();
    eventXML += "\"/>\n";

    eventXML += " <" + InteractionEventConst::xmlTagAttribute() +" " + InteractionEventConst::xmlParameterName() + "=\"" + InteractionEventConst::xmlEventPropertyScrollDirection() + "\" ";
    eventXML += InteractionEventConst::xmlParameterValue() + "=\"";
    eventXML += delta < 0 ? "DOWN" : "UP";
    eventXML += "\"/>\n";
  }

  // Renderer name
  eventXML += " <" + InteractionEventConst::xmlTagAttribute() +" " + InteractionEventConst::xmlParameterName() + "=\"" + InteractionEventConst::xmlEventPropertyRendererName() + "\" ";
  eventXML += InteractionEventConst::xmlParameterValue() + "=\"";
  eventXML += event->GetSender()->GetName();
  eventXML += "\"/>\n";

  // closing tag:
  eventXML += "</" + InteractionEventConst::xmlTagEventVariant() +  ">";
  return eventXML;
}
