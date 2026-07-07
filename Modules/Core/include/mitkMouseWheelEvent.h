/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMouseWheelEvent_h
#define mitkMouseWheelEvent_h

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
   * \brief Interaction event class representing a mouse wheel scroll.
   *
   * Stores the mouse position, the scroll delta, the combined state of all
   * mouse buttons, and any active modifier keys. Two MouseWheelEvent instances
   * are considered equal if they scroll in the same direction (same sign of delta).
   *
   * \note A click with the middle mouse button should be handled with MousePressEvent,
   * not MouseWheelEvent.
   *
   * \ingroup Interaction
   * \sa MousePressEvent
   * \sa InteractionPositionEvent
   */
  class MITKCORE_EXPORT MouseWheelEvent : public InteractionPositionEvent
  {
  public:
    mitkClassMacro(MouseWheelEvent, InteractionPositionEvent)
      mitkNewMacro5Param(Self, BaseRenderer *, const Point2D &, MouseButtons, ModifierKeys, int)

    /** \brief Get the active modifier keys (Shift, Ctrl, Alt, etc.). */
    ModifierKeys GetModifiers() const;

    /** \brief Get the combined state of all mouse buttons at the time of the event. */
    MouseButtons GetButtonStates() const;

    /** \brief Set the active modifier keys. */
    void SetModifiers(ModifierKeys modifiers);

    /** \brief Set the combined mouse button state. */
    void SetButtonStates(MouseButtons buttons);

    /**
     * \brief Get the wheel scroll delta.
     * \return Positive values indicate scrolling up/forward, negative values down/backward.
     */
    int GetWheelDelta() const;

    /**
     * \brief Set the wheel scroll delta.
     * \param delta Positive values indicate scrolling up/forward, negative values down/backward.
     */
    void SetWheelDelta(int delta);

    /**
     * \brief Check if the given event is an instance of this class or a subclass.
     * \return true if \p baseClass is a MouseWheelEvent.
     */
    bool IsSuperClassOf(const InteractionEvent::Pointer &baseClass) const override;

  protected:
    MouseWheelEvent(BaseRenderer * = nullptr,
                    const Point2D &mousePosition = Point2D(),
                    MouseButtons buttonStates = NoButton,
                    ModifierKeys modifiers = NoKey,
                    int wheelDelta = 0);
    ~MouseWheelEvent() override;

    /**
     * \brief Compare this event to another for equality.
     *
     * Two MouseWheelEvent instances are equal if they scroll in the
     * same direction (same sign of wheel delta), have the same modifier keys,
     * button states, and position.
     */
    bool IsEqual(const InteractionEvent &) const override;

  private:
    int m_WheelDelta;
    MouseButtons m_ButtonStates;
    ModifierKeys m_Modifiers;
  };
} /* namespace mitk */

#endif
