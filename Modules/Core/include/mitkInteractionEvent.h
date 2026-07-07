/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkInteractionEvent_h
#define mitkInteractionEvent_h

#include <itkLightObject.h>
#include <itkObjectFactory.h>
#include <mitkBaseRenderer.h>
#include <mitkCommon.h>

#include <MitkCoreExports.h>
#include <string>

namespace mitk
{
  /**
   * \brief Base class for all interaction events in MITK.
   *
   * InteractionEvent is the root of the interaction event hierarchy. Every
   * interaction event carries a reference to the BaseRenderer that generated it
   * (the "sender"). Subclasses add position information (InteractionPositionEvent),
   * keyboard information (InteractionKeyEvent), or mouse button/wheel details.
   *
   * Equality between events is checked via operator== which delegates to IsEqual().
   * Two events match if they agree on all attributes relevant for state machine
   * transitions (e.g. mouse button, modifiers) while ignoring attributes like
   * the exact pointer position.
   *
   * \sa InteractionPositionEvent
   * \sa InteractionKeyEvent
   * \sa EventStateMachine
   * \sa Dispatcher
   * \ingroup Interaction
   */
  class MITKCORE_EXPORT InteractionEvent : public itk::LightObject
  {
  public:
    mitkClassMacroItkParent(InteractionEvent, itk::LightObject);
    mitkNewMacro1Param(Self, BaseRenderer*);

    /**
     * \brief Set the BaseRenderer that sent this event.
     * \param[in] sender Pointer to the sending BaseRenderer.
     */
    void SetSender(BaseRenderer *sender);

    /**
     * \brief Get the BaseRenderer that sent this event.
     * \return Pointer to the sending BaseRenderer.
     */
    BaseRenderer *GetSender() const;

    /**
     * \brief Check if the provided event is an instance of this class or a derived class.
     *
     * This up-cast check supports polymorphism at the state machine pattern (XML) level,
     * allowing transitions to match on base event types.
     *
     * \param[in] baseClass The event to check.
     * \return true if baseClass is an InteractionEvent or derived from it.
     */
    virtual bool IsSuperClassOf(const InteractionEvent::Pointer &baseClass) const;

    /**
     * \brief Enumeration of mouse button states.
     *
     * Values can be combined with bitwise OR to represent multiple pressed buttons.
     */
    enum MouseButtons
    {
      NoButton = 0x0000,         ///< No mouse button.
      LeftMouseButton = 0x0001,  ///< Left mouse button.
      RightMouseButton = 0x0002, ///< Right mouse button.
      MiddleMouseButton = 0x0004 ///< Middle mouse button.
    };

    /**
     * \brief Enumeration of keyboard modifier keys.
     *
     * Values can be combined with bitwise OR to represent multiple held modifiers.
     */
    enum ModifierKeys
    {
      NoKey = 0x0000,      ///< No modifier key.
      ShiftKey = 0x0100,   ///< Shift key.
      ControlKey = 0x0200, ///< Control (Ctrl) key.
      AltKey = 0x0400      ///< Alt key.
    };

    /** \brief String constants for special keys used in key events and state machine patterns. */
    ///@{
    static const std::string KeyEsc;        ///< "Escape" key constant.
    static const std::string KeyEnter;      ///< "Enter" key constant.
    static const std::string KeyReturn;     ///< "Return" key constant.
    static const std::string KeyDelete;     ///< "Delete" key constant.
    static const std::string KeyArrowUp;    ///< "ArrowUp" key constant.
    static const std::string KeyArrowDown;  ///< "ArrowDown" key constant.
    static const std::string KeyArrowLeft;  ///< "ArrowLeft" key constant.
    static const std::string KeyArrowRight; ///< "ArrowRight" key constant.

    static const std::string KeyF1;  ///< "F1" key constant.
    static const std::string KeyF2;  ///< "F2" key constant.
    static const std::string KeyF3;  ///< "F3" key constant.
    static const std::string KeyF4;  ///< "F4" key constant.
    static const std::string KeyF5;  ///< "F5" key constant.
    static const std::string KeyF6;  ///< "F6" key constant.
    static const std::string KeyF7;  ///< "F7" key constant.
    static const std::string KeyF8;  ///< "F8" key constant.
    static const std::string KeyF9;  ///< "F9" key constant.
    static const std::string KeyF10; ///< "F10" key constant.
    static const std::string KeyF11; ///< "F11" key constant.
    static const std::string KeyF12; ///< "F12" key constant.

    static const std::string KeyPos1;     ///< "Pos1" (Home) key constant.
    static const std::string KeyEnd;      ///< "End" key constant.
    static const std::string KeyInsert;   ///< "Insert" key constant.
    static const std::string KeyPageUp;   ///< "PageUp" key constant.
    static const std::string KeyPageDown; ///< "PageDown" key constant.
    static const std::string KeySpace;    ///< "Space" key constant.
    ///@}

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

#endif
