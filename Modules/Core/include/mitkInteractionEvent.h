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

#ifndef MITKINTERACTIONEVENT_H_
#define MITKINTERACTIONEVENT_H_

#include "itkLightObject.h"
#include "itkObjectFactory.h"
#include "mitkBaseRenderer.h"
#include "mitkCommon.h"

#include <MitkCoreExports.h>
#include <string>

namespace mitk
{
  class MITKCORE_EXPORT InteractionEvent : public itk::LightObject
  {
  public:
    mitkClassMacroItkParent(InteractionEvent, itk::LightObject) mitkNewMacro1Param(Self, BaseRenderer *)

      void SetSender(BaseRenderer *sender);
    BaseRenderer *GetSender() const;

    /**
     * This class implements an up cast to check if the provided baseClass object is derived from this class.
     * This function is used to support polymorphism on state machine pattern (XML) level.
     */
    virtual bool IsSuperClassOf(const InteractionEvent::Pointer &baseClass) const;

    /**
     * Mouse/keyboard state values
     */
    enum MouseButtons
    {
      NoButton = 0x0000,
      LeftMouseButton = 0x0001,
      RightMouseButton = 0x0002,
      MiddleMouseButton = 0x0004
    };

    enum ModifierKeys
    {
      NoKey = 0x0000,
      ShiftKey = 0x0100,
      ControlKey = 0x0200,
      AltKey = 0x0400
    };

    /**
     * KeyConstants Constants for special keys
     */
    // Special Keys
    static const std::string KeyEsc;        // = "Escape";
    static const std::string KeyEnter;      // = "Enter";
    static const std::string KeyReturn;     // = "Return";
    static const std::string KeyDelete;     // = "Delete";
    static const std::string KeyArrowUp;    // = "ArrowUp";
    static const std::string KeyArrowDown;  // = "ArrowDown";
    static const std::string KeyArrowLeft;  // = "ArrowLeft";
    static const std::string KeyArrowRight; // = "ArrowRight";

    static const std::string KeyF1;  // = "F1";
    static const std::string KeyF2;  // = "F2";
    static const std::string KeyF3;  // = "F3";
    static const std::string KeyF4;  // = "F4";
    static const std::string KeyF5;  // = "F5";
    static const std::string KeyF6;  // = "F6";
    static const std::string KeyF7;  // = "F7";
    static const std::string KeyF8;  // = "F8";
    static const std::string KeyF9;  // = "F9";
    static const std::string KeyF10; // = "F10";
    static const std::string KeyF11; // = "F11";
    static const std::string KeyF12; // = "F12";

    static const std::string KeyPos1;     // = "Pos1";
    static const std::string KeyEnd;      // = "End";
    static const std::string KeyInsert;   // = "Insert";
    static const std::string KeyPageUp;   // = "PageUp";
    static const std::string KeyPageDown; // = "PageDown";
    static const std::string KeySpace;    // = "Space";
    // End special keys

  protected:
    InteractionEvent(BaseRenderer *);
    ~InteractionEvent() override;

    friend MITKCORE_EXPORT bool operator==(const InteractionEvent &, const InteractionEvent &);
    virtual bool IsEqual(const InteractionEvent &other) const;

  private:
    BaseRenderer *m_Sender;
  };

  /**
   * Implementation of equality for event classes.
   * Equality does \b not mean an exact copy or pointer equality.
   *
   * A match is determined by agreement in all attributes that are necessary to describe
   * the event for a state machine transition.
   * E.g. for a mouse event press event, it is important which modifiers are used,
   * which mouse button was used to triggered the event, but the mouse position is irrelevant.
   */
  MITKCORE_EXPORT bool operator==(const InteractionEvent &a, const InteractionEvent &b);
  MITKCORE_EXPORT bool operator!=(const InteractionEvent &a, const InteractionEvent &b);

  /*
   * Allow bitwise OR operation on enums.
   */
  inline InteractionEvent::MouseButtons operator|(InteractionEvent::MouseButtons a, InteractionEvent::MouseButtons b)
  {
    return static_cast<InteractionEvent::MouseButtons>(static_cast<int>(a) | static_cast<int>(b));
  }

  inline InteractionEvent::MouseButtons &operator|=(InteractionEvent::MouseButtons &a,
                                                    InteractionEvent::MouseButtons &b)
  {
    a = static_cast<InteractionEvent::MouseButtons>(static_cast<int>(a) | static_cast<int>(b));
    return a;
  }

  inline InteractionEvent::ModifierKeys operator|(InteractionEvent::ModifierKeys a, InteractionEvent::ModifierKeys b)
  {
    return static_cast<InteractionEvent::ModifierKeys>(static_cast<int>(a) | static_cast<int>(b));
  }

  inline InteractionEvent::ModifierKeys &operator|=(InteractionEvent::ModifierKeys &a,
                                                    InteractionEvent::ModifierKeys &b)
  {
    a = static_cast<InteractionEvent::ModifierKeys>(static_cast<int>(a) | static_cast<int>(b));
    return a;
  }

} /* namespace mitk */

#endif /* MITKINTERACTIONEVENT_H_ */
