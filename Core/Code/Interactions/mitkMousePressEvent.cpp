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

#include "mitkException.h"
#include "mitkMousePressEvent.h"

mitk::MousePressEvent::MousePressEvent(mitk::BaseRenderer* baseRenderer = NULL, mitk::Point2D mousePosition = NULL, mitk::MouseButtons buttonStates =
    NoButton, mitk::ModifierKeys modifiers = NoKey, mitk::MouseButtons eventButton = NoButton) :
    InteractionPositionEvent(baseRenderer, mousePosition, buttonStates, modifiers, "MousePressEvent"), m_EventButton(eventButton)
{
}

mitk::MouseButtons mitk::MousePressEvent::GetEventButton() const
{
  return m_EventButton;
}

void mitk::MousePressEvent::SetEventButton(MouseButtons buttons)
{
  m_EventButton = buttons;
}

mitk::MousePressEvent::~MousePressEvent()
{
}

bool mitk::MousePressEvent::isEqual(mitk::InteractionEvent::Pointer interactionEvent)
{
  mitk::MousePressEvent* mpe = dynamic_cast<mitk::MousePressEvent*>(interactionEvent.GetPointer());
  if (mpe == NULL)
  {
    return false;
  }
  return (this->GetEventButton() == mpe->GetEventButton() && this->GetModifiers() == mpe->GetModifiers()
      && this->GetButtonStates() == mpe->GetButtonStates());
}

