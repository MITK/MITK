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

#ifndef MITKINTERACTEVENTCONST_H
#define MITKINTERACTEVENTCONST_H

//##Documentation
//## @file mitkInteractionEventConst.h
//## @brief Constants to describe Mouse Events and special Key Events.

namespace mitk
{

//##ButtonState
// Mouse/keyboard state values
  enum MouseButtons
  {
    NoButton = 0x0000,
    LeftMouseButton = 0x0001,
    RightMouseButton = 0x0002,
    MiddleMouseButton = 0x0004,
  };

  enum ModifierKeys
  {
    NoKey = 0x0000,
    ShiftKey = 0x0100,
    ControlKey = 0x0200,
    AltKey = 0x0400
  };

  /*
   * Allow bitwise OR operation on enums.
   */
  inline MouseButtons operator|(MouseButtons a, MouseButtons b)
  {
    return static_cast<MouseButtons>(static_cast<int>(a) | static_cast<int>(b));
  }
  // TODO: FixME not working!
  inline MouseButtons operator|=(MouseButtons a, MouseButtons b)
  {
    return static_cast<MouseButtons>(static_cast<int>(a) | static_cast<int>(b));
  }

  inline ModifierKeys operator|(ModifierKeys a, ModifierKeys b)
  {
    return static_cast<ModifierKeys>(static_cast<int>(a) | static_cast<int>(b));
  }

// TODO: FixME not working!
  inline ModifierKeys operator|=(ModifierKeys a, ModifierKeys b)
  {
    return static_cast<ModifierKeys>(static_cast<int>(a) | static_cast<int>(b));
  }

  /**
   * KeyConstants Constants for special keys
   */
  // Special Keys
  const std::string KeyEsc = "Escape";
  const std::string KeyEnter = "Enter";
  const std::string KeyReturn = "Return";
  const std::string KeyDelete = "Delete";
  const std::string KeyArrowUp = "ArrowUp";
  const std::string KeyArrowDown = "ArrowDown";
  const std::string KeyArrowLeft = "ArrowLeft";
  const std::string KeyArrowRight = "ArrowRight";

  const std::string KeyF1 = "F1";
  const std::string KeyF2 = "F2";
  const std::string KeyF3 = "F3";
  const std::string KeyF4 = "F4";
  const std::string KeyF5 = "F5";
  const std::string KeyF6 = "F6";
  const std::string KeyF7 = "F7";
  const std::string KeyF8 = "F8";
  const std::string KeyF9 = "F9";
  const std::string KeyF10 = "F10";
  const std::string KeyF11 = "F11";
  const std::string KeyF12 = "F12";

  const std::string KeyPos1 = "Pos1";
  const std::string KeyEend = "End";
  const std::string KeyInsert = "Insert";
  const std::string KeyPageUp = "PageUp";
  const std::string KeyPageDown = "PageDown";
  // End special keys

  // XML Tags
  const std::string xmlTagConfigRoot = "config";
  const std::string xmlTagParam = "param";
  const std::string xmlTagEventVariant = "event_variant";
  const std::string xmlTagAttribute = "attribute";

  // XML Param
  const std::string xmlParameterName = "name";
  const std::string xmlParameterValue = "value";
  const std::string xmlParameterEventVariant = "event_variant";
  const std::string xmlParameterEventClass = "class";

  // Event Description
  const std::string xmlEventPropertyModifier = "Modifiers";
  const std::string xmlEventPropertyEventButton = "EventButton";
  const std::string xmlEventPropertyButtonState = "ButtonState";
  const std::string xmlEventPropertyKey = "Key";
  const std::string xmlEventPropertyScrollDirection = "ScrollDirection";
  const std::string xmlEventPropertySignalName = "SignalName";

  // Predefined internal events/signals
  const std::string IntDeactivateMe = "DeactivateMe";
  const std::string IntLeaveWidget = "LeaveWidget";
  const std::string IntEnterWidget = "EnterWidget";

}     //namespace mitk
#endif //ifndef MITKINTERACTEVENTCONST_H
