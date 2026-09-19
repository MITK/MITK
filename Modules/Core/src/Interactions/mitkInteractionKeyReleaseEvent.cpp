/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkInteractionKeyReleaseEvent.h>

mitk::InteractionKeyReleaseEvent::InteractionKeyReleaseEvent(BaseRenderer *baseRenderer,
                                                             const std::string &key,
                                                             ModifierKeys modifiers)
  : InteractionEvent(baseRenderer), m_Key(key), m_Modifiers(modifiers)
{
}

mitk::InteractionKeyReleaseEvent::~InteractionKeyReleaseEvent()
{
}

mitk::InteractionEvent::ModifierKeys mitk::InteractionKeyReleaseEvent::GetModifiers() const
{
  return m_Modifiers;
}

std::string mitk::InteractionKeyReleaseEvent::GetKey() const
{
  return m_Key;
}

bool mitk::InteractionKeyReleaseEvent::IsEqual(const InteractionEvent &interactionEvent) const
{
  const auto &keyEvent = static_cast<const Self &>(interactionEvent);
  return this->GetModifiers() == keyEvent.GetModifiers() && this->GetKey() == keyEvent.GetKey() &&
         Superclass::IsEqual(interactionEvent);
}

bool mitk::InteractionKeyReleaseEvent::IsSuperClassOf(const InteractionEvent::Pointer &baseClass) const
{
  return dynamic_cast<InteractionKeyReleaseEvent *>(baseClass.GetPointer()) != nullptr;
}
