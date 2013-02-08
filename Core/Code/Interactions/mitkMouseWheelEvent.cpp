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

mitk::MouseWheelEvent::MouseWheelEvent(BaseRenderer* baseRenderer = NULL,
    Point2D mousePosition = NULL,
    MouseButtons buttonStates = NoButton,
    ModifierKeys modifiers = NoKey,
    int wheelDelta = 0)
: InteractionPositionEvent(baseRenderer, mousePosition, "MouseWheelEvent")
, m_WheelDelta(wheelDelta)
, m_ButtonStates(buttonStates)
, m_Modifiers(modifiers)
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

mitk::ModifierKeys mitk::MouseWheelEvent::GetModifiers() const
{
  return m_Modifiers;
}

mitk::MouseButtons mitk::MouseWheelEvent::GetButtonStates() const
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

bool mitk::MouseWheelEvent::MatchesTemplate(mitk::InteractionEvent::Pointer interactionEvent)
{
  const mitk::MouseWheelEvent* mwe = dynamic_cast<const MouseWheelEvent*>(interactionEvent.GetPointer());
  if (mwe == NULL)
  {
    return false;
  }
  return ((this->GetWheelDelta() * mwe->GetWheelDelta() > 0) // Consider WheelEvents to be equal if the scrolling is done in the same direction.
  && this->GetModifiers() == mwe->GetModifiers() && this->GetButtonStates() == mwe->GetButtonStates());
}
