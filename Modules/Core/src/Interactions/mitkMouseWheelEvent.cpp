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

#include "mitkMouseWheelEvent.h"

mitk::MouseWheelEvent::MouseWheelEvent(BaseRenderer *baseRenderer,
                                       const Point2D &mousePosition,
                                       MouseButtons buttonStates,
                                       ModifierKeys modifiers,
                                       int wheelDelta)
  : InteractionPositionEvent(baseRenderer, mousePosition),
    m_WheelDelta(wheelDelta),
    m_ButtonStates(buttonStates),
    m_Modifiers(modifiers)
{
}

int mitk::MouseWheelEvent::GetWheelDelta() const
{
  return m_WheelDelta;
}

void mitk::MouseWheelEvent::SetWheelDelta(int delta)
{
  m_WheelDelta = delta;
}

mitk::InteractionEvent::ModifierKeys mitk::MouseWheelEvent::GetModifiers() const
{
  return m_Modifiers;
}

mitk::InteractionEvent::MouseButtons mitk::MouseWheelEvent::GetButtonStates() const
{
  return m_ButtonStates;
}

void mitk::MouseWheelEvent::SetModifiers(ModifierKeys modifiers)
{
  m_Modifiers = modifiers;
}

void mitk::MouseWheelEvent::SetButtonStates(MouseButtons buttons)
{
  m_ButtonStates = buttons;
}

mitk::MouseWheelEvent::~MouseWheelEvent()
{
}

bool mitk::MouseWheelEvent::IsEqual(const mitk::InteractionEvent &interactionEvent) const
{
  const auto &mwe = static_cast<const MouseWheelEvent &>(interactionEvent);
  return ((this->GetWheelDelta() * mwe.GetWheelDelta() >=
           0) // Consider WheelEvents to be equal if the scrolling is done in the same direction.
          &&
          this->GetModifiers() == mwe.GetModifiers() && this->GetButtonStates() == mwe.GetButtonStates() &&
          Superclass::IsEqual(interactionEvent));
}

bool mitk::MouseWheelEvent::IsSuperClassOf(const InteractionEvent::Pointer &baseClass) const
{
  return (dynamic_cast<MouseWheelEvent *>(baseClass.GetPointer()) != nullptr);
}
