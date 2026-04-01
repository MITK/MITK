/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMouseReleaseEvent_h
#define mitkMouseReleaseEvent_h

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <mitkBaseRenderer.h>
#include <mitkCommon.h>
#include <mitkInteractionEvent.h>
#include <mitkInteractionEventConst.h>
#include <mitkInteractionPositionEvent.h>

#include <MitkCoreExports.h>

namespace mitk
{
  /**
   * \brief Interaction event class representing a mouse button release.
   *
   * Stores the mouse position, the button that was released,
   * the combined state of all mouse buttons, and any active modifier keys.
   *
   * \ingroup Interaction
   * \sa MousePressEvent
   * \sa MouseDoubleClickEvent
   * \sa InteractionPositionEvent
   */
  class MITKCORE_EXPORT MouseReleaseEvent : public InteractionPositionEvent
  {
  public:
    mitkClassMacro(MouseReleaseEvent, InteractionPositionEvent)
      mitkNewMacro5Param(Self, BaseRenderer *, const Point2D &, MouseButtons, ModifierKeys, MouseButtons)

    /** \brief Get the active modifier keys (Shift, Ctrl, Alt, etc.). */
    ModifierKeys GetModifiers() const;

    /** \brief Get the combined state of all mouse buttons at the time of the event. */
    MouseButtons GetButtonStates() const;

    /** \brief Set the active modifier keys. */
    void SetModifiers(ModifierKeys modifiers);

    /** \brief Set the combined mouse button state. */
    void SetButtonStates(MouseButtons buttons);

    /** \brief Get the specific mouse button that triggered this release event. */
    MouseButtons GetEventButton() const;

    /** \brief Set the mouse button that triggered this release event. */
    void SetEventButton(MouseButtons buttons);

    /**
     * \brief Check if the given event is an instance of this class or a subclass.
     * \return true if \p baseClass is a MouseReleaseEvent.
     */
    bool IsSuperClassOf(const InteractionEvent::Pointer &baseClass) const override;

  protected:
    MouseReleaseEvent(BaseRenderer *,
                      const mitk::Point2D &mousePosition = Point2D(),
                      MouseButtons buttonStates = NoButton,
                      ModifierKeys modifiers = NoKey,
                      MouseButtons eventButton = NoButton);
    ~MouseReleaseEvent() override;

    /**
     * \brief Compare this event to another for equality.
     *
     * Two MouseReleaseEvent instances are equal if they share the same
     * event button, modifier keys, button states, and position.
     */
    bool IsEqual(const InteractionEvent &) const override;

  private:
    MouseButtons m_EventButton;
    MouseButtons m_ButtonStates;
    ModifierKeys m_Modifiers;
  };
} /* namespace mitk */

#endif
