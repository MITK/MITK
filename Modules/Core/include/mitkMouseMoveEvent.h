/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMouseMoveEvent_h
#define mitkMouseMoveEvent_h
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
   * \brief Interaction event class representing mouse movement.
   *
   * Stores the current mouse position, the combined state of all pressed
   * mouse buttons, and any active modifier keys during the movement.
   *
   * \ingroup Interaction
   * \sa MousePressEvent
   * \sa MouseReleaseEvent
   * \sa InteractionPositionEvent
   */
  class MITKCORE_EXPORT MouseMoveEvent : public InteractionPositionEvent
  {
  public:
    mitkClassMacro(MouseMoveEvent, InteractionPositionEvent);
    mitkNewMacro4Param(Self, BaseRenderer *, const Point2D &, MouseButtons, ModifierKeys);

    /** \brief Get the active modifier keys (Shift, Ctrl, Alt, etc.). */
    ModifierKeys GetModifiers() const;

    /** \brief Get the combined state of all mouse buttons at the time of the event. */
    MouseButtons GetButtonStates() const;

    /** \brief Set the active modifier keys. */
    void SetModifiers(ModifierKeys modifiers);

    /** \brief Set the combined mouse button state. */
    void SetButtonStates(MouseButtons buttons);

    /**
     * \brief Check if the given event is an instance of this class or a subclass.
     * \return true if \p baseClass is a MouseMoveEvent.
     */
    bool IsSuperClassOf(const InteractionEvent::Pointer &baseClass) const override;

  protected:
    MouseMoveEvent(BaseRenderer *,
                   const Point2D & = Point2D(),
                   MouseButtons buttonStates = NoButton,
                   ModifierKeys modifiers = NoKey);
    ~MouseMoveEvent() override;

    /**
     * \brief Compare this event to another for equality.
     *
     * Two MouseMoveEvent instances are equal if they share the same
     * modifier keys, button states, and position.
     */
    bool IsEqual(const InteractionEvent &) const override;

  private:
    MouseButtons m_ButtonStates;
    ModifierKeys m_Modifiers;
  };
} /* namespace mitk */

#endif
