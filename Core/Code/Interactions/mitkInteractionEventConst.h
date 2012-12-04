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

  inline MouseButtons operator|=(MouseButtons a, MouseButtons b)
  {
    return static_cast<MouseButtons>(static_cast<int>(a) | static_cast<int>(b));
  }

  inline ModifierKeys operator|(ModifierKeys a, ModifierKeys b)
  {
    return static_cast<ModifierKeys>(static_cast<int>(a) | static_cast<int>(b));
  }

  inline ModifierKeys operator|=(ModifierKeys a, ModifierKeys b)
  {
    return static_cast<ModifierKeys>(static_cast<int>(a) | static_cast<int>(b));
  }

}     //namespace mitk
#endif //ifndef MITKINTERACTEVENTCONST_H
