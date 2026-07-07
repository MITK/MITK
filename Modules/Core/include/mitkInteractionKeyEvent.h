/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkInteractionKeyEvent_h
#define mitkInteractionKeyEvent_h

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <mitkBaseRenderer.h>
#include <mitkCommon.h>
#include <mitkInteractionEvent.h>
#include <mitkInteractionEventConst.h>
#include <mitkInteractionPositionEvent.h>
#include <cstring>

#include <MitkCoreExports.h>

/*
 * Note: A Click with the MiddleButton is to be handled with MousePressEvents
 */
namespace mitk
{
  /**
   * \brief Represents a keyboard interaction event.
   *
   * Stores the pressed key (as a string, including special key constants from
   * InteractionEvent) and the modifier keys held during the event. Two
   * InteractionKeyEvents are considered equal if they have the same key
   * and the same modifiers.
   *
   * \sa InteractionEvent
   * \sa InteractionPositionEvent
   * \ingroup Interaction
   */
  class MITKCORE_EXPORT InteractionKeyEvent : public InteractionEvent
  {
  public:
    mitkClassMacro(InteractionKeyEvent, InteractionEvent)
    mitkNewMacro3Param(Self, BaseRenderer*, const std::string&, ModifierKeys);

    /**
     * \brief Check if the provided event is an InteractionKeyEvent or derived.
     * \param[in] baseClass The event to check.
     * \return true if baseClass is an InteractionKeyEvent or derived from it.
     */
    bool IsSuperClassOf(const InteractionEvent::Pointer &baseClass) const override;

    /**
     * \brief Get the modifier keys held during this key event.
     * \return The modifier keys as a ModifierKeys bitmask.
     */
    ModifierKeys GetModifiers() const;

    /**
     * \brief Get the key that was pressed.
     * \return The key as a string. For special keys, this matches the KeyXxx constants.
     */
    std::string GetKey() const;

  protected:
    InteractionKeyEvent(BaseRenderer *, const std::string &key, ModifierKeys modifiers);
    ~InteractionKeyEvent() override;

    bool IsEqual(const InteractionEvent &) const override;

  private:
    std::string m_Key;
    ModifierKeys m_Modifiers;
  };
} /* namespace mitk */

#endif
