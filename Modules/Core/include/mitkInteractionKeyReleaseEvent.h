/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkInteractionKeyReleaseEvent_h
#define mitkInteractionKeyReleaseEvent_h

#include <mitkInteractionEvent.h>

#include <MitkCoreExports.h>

#include <string>

namespace mitk
{
  /**
   * \brief Represents the release of a keyboard key.
   *
   * Counterpart of InteractionKeyEvent, which represents a key press. Stores
   * the released key (as a string, including special key constants from
   * InteractionEvent) and the modifier keys held during the event. Two
   * InteractionKeyReleaseEvents are considered equal if they have the same
   * key and the same modifiers.
   *
   * Unlike key presses, key releases have no implicit "Std" + key event
   * variant. A state machine configuration has to declare an event variant
   * of this class explicitly.
   *
   * \sa InteractionKeyEvent
   * \sa InteractionEvent
   * \ingroup Interaction
   */
  class MITKCORE_EXPORT InteractionKeyReleaseEvent : public InteractionEvent
  {
  public:
    mitkClassMacro(InteractionKeyReleaseEvent, InteractionEvent);
    mitkNewMacro3Param(Self, BaseRenderer*, const std::string&, ModifierKeys);

    /**
     * \brief Check if the provided event is an InteractionKeyReleaseEvent or derived.
     * \param[in] baseClass The event to check.
     * \return true if baseClass is an InteractionKeyReleaseEvent or derived from it.
     */
    bool IsSuperClassOf(const InteractionEvent::Pointer &baseClass) const override;

    /**
     * \brief Get the modifier keys held during this key event.
     * \return The modifier keys as a ModifierKeys bitmask.
     */
    ModifierKeys GetModifiers() const;

    /**
     * \brief Get the key that was released.
     * \return The key as a string. For special keys, this matches the KeyXxx constants.
     */
    std::string GetKey() const;

  protected:
    InteractionKeyReleaseEvent(BaseRenderer *, const std::string &key, ModifierKeys modifiers);
    ~InteractionKeyReleaseEvent() override;

    bool IsEqual(const InteractionEvent &) const override;

  private:
    std::string m_Key;
    ModifierKeys m_Modifiers;
  };
} /* namespace mitk */

#endif
