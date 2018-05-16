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

#include "mitkMousePressEvent.h"
#include "mitkException.h"

mitk::MousePressEvent::MousePressEvent(mitk::BaseRenderer *baseRenderer,
                                       const mitk::Point2D &mousePosition,
                                       MouseButtons buttonStates,
                                       ModifierKeys modifiers,
                                       MouseButtons eventButton)
  : InteractionPositionEvent(baseRenderer, mousePosition),
    m_EventButton(eventButton),
    m_ButtonStates(buttonStates),
    m_Modifiers(modifiers)
{
}

mitk::InteractionEvent::MouseButtons mitk::MousePressEvent::GetEventButton() const
{
  return m_EventButton;
}

void mitk::MousePressEvent::SetEventButton(MouseButtons buttons)
{
  m_EventButton = buttons;
}

mitk::InteractionEvent::ModifierKeys mitk::MousePressEvent::GetModifiers() const
{
  return m_Modifiers;
}

mitk::InteractionEvent::MouseButtons mitk::MousePressEvent::GetButtonStates() const
{
  return m_ButtonStates;
}

void mitk::MousePressEvent::SetModifiers(ModifierKeys modifiers)
{
  m_Modifiers = modifiers;
}

void mitk::MousePressEvent::SetButtonStates(MouseButtons buttons)
{
  m_ButtonStates = buttons;
}

mitk::MousePressEvent::~MousePressEvent()
{
}

bool mitk::MousePressEvent::IsEqual(const mitk::InteractionEvent &interactionEvent) const
{
  const auto &mpe = static_cast<const mitk::MousePressEvent &>(interactionEvent);
  return (this->GetEventButton() == mpe.GetEventButton() && this->GetModifiers() == mpe.GetModifiers() &&
          this->GetButtonStates() == mpe.GetButtonStates() && Superclass::IsEqual(interactionEvent));
}

bool mitk::MousePressEvent::IsSuperClassOf(const InteractionEvent::Pointer &baseClass) const
{
  return (dynamic_cast<MousePressEvent *>(baseClass.GetPointer()) != nullptr);
}
