/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMouseMoveEvent.h"
#include "mitkException.h"

mitk::MouseMoveEvent::MouseMoveEvent(mitk::BaseRenderer *baseRenderer,
                                     const mitk::Point2D &mousePosition,
                                     MouseButtons buttonStates,
                                     ModifierKeys modifiers)
  : InteractionPositionEvent(baseRenderer, mousePosition), m_ButtonStates(buttonStates), m_Modifiers(modifiers)
{
}

mitk::InteractionEvent::ModifierKeys mitk::MouseMoveEvent::GetModifiers() const
{
  return m_Modifiers;
}

mitk::InteractionEvent::MouseButtons mitk::MouseMoveEvent::GetButtonStates() const
{
  return m_ButtonStates;
}

void mitk::MouseMoveEvent::SetModifiers(ModifierKeys modifiers)
{
  m_Modifiers = modifiers;
}

void mitk::MouseMoveEvent::SetButtonStates(MouseButtons buttons)
{
  m_ButtonStates = buttons;
}

mitk::MouseMoveEvent::~MouseMoveEvent()
{
}

bool mitk::MouseMoveEvent::IsEqual(const mitk::InteractionEvent &interactionEvent) const
{
  const auto &mpe = static_cast<const mitk::MouseMoveEvent &>(interactionEvent);
  return (this->GetModifiers() == mpe.GetModifiers() && this->GetButtonStates() == mpe.GetButtonStates() &&
          Superclass::IsEqual(interactionEvent));
}

bool mitk::MouseMoveEvent::IsSuperClassOf(const InteractionEvent::Pointer &baseClass) const
{
  return (dynamic_cast<MouseMoveEvent *>(baseClass.GetPointer()) != nullptr);
}
