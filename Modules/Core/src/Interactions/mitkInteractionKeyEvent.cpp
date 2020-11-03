/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkInteractionKeyEvent.h"

mitk::InteractionKeyEvent::InteractionKeyEvent(mitk::BaseRenderer *baseRenderer,
                                               const std::string &key,
                                               ModifierKeys modifiers = ControlKey)
  : InteractionEvent(baseRenderer), m_Key(key), m_Modifiers(modifiers)
{
}

mitk::InteractionEvent::ModifierKeys mitk::InteractionKeyEvent::GetModifiers() const
{
  return m_Modifiers;
}

std::string mitk::InteractionKeyEvent::GetKey() const
{
  return m_Key;
}

mitk::InteractionKeyEvent::~InteractionKeyEvent()
{
}

bool mitk::InteractionKeyEvent::IsEqual(const mitk::InteractionEvent &interactionEvent) const
{
  const auto &keyEvent = static_cast<const Self &>(interactionEvent);
  return (this->GetModifiers() == keyEvent.GetModifiers() && this->GetKey() == keyEvent.GetKey() &&
          Superclass::IsEqual(interactionEvent));
}

bool mitk::InteractionKeyEvent::IsSuperClassOf(const InteractionEvent::Pointer &baseClass) const
{
  return (dynamic_cast<InteractionKeyEvent *>(baseClass.GetPointer()) != nullptr);
}
