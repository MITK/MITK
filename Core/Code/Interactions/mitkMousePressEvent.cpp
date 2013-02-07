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

mitk::MousePressEvent::MousePressEvent(mitk::BaseRenderer* baseRenderer = NULL,
    mitk::Point2D mousePosition = NULL,
    mitk::MouseButtons buttonStates = NoButton,
    mitk::ModifierKeys modifiers = NoKey,
    mitk::MouseButtons eventButton = NoButton) :
    InteractionPositionEvent(baseRenderer, mousePosition, "MousePressEvent"), m_EventButton(eventButton), m_ButtonStates(buttonStates), m_Modifiers(
        modifiers)
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

bool mitk::MousePressEvent::IsSuperClassOf(InteractionEvent::Pointer baseClass)
{
  MousePressEvent* event = dynamic_cast<MousePressEvent*>(baseClass.GetPointer());
  if (event != NULL)
  {
    return true;
  }
  return false;
}

mitk::ModifierKeys mitk::MousePressEvent::GetModifiers() const
{
  return m_Modifiers;
}

mitk::MouseButtons mitk::MousePressEvent::GetButtonStates() const
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

bool mitk::MousePressEvent::MatchesTemplate(mitk::InteractionEvent::Pointer interactionEvent)
{
  mitk::MousePressEvent* mpe = dynamic_cast<mitk::MousePressEvent*>(interactionEvent.GetPointer());
  if (mpe == NULL)
  {
    return false;
  }
  return (this->GetEventButton() == mpe->GetEventButton() && this->GetModifiers() == mpe->GetModifiers()
      && this->GetButtonStates() == mpe->GetButtonStates());
}
