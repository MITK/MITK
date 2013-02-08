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
#include "mitkMouseReleaseEvent.h"

mitk::MouseReleaseEvent::MouseReleaseEvent(mitk::BaseRenderer* baseRenderer = NULL,
    mitk::Point2D mousePosition = NULL,
    mitk::MouseButtons buttonStates = NoButton,
    mitk::ModifierKeys modifiers = NoKey,
    mitk::MouseButtons eventButton = NoButton)
: InteractionPositionEvent(baseRenderer, mousePosition, "MouseReleaseEvent")
, m_EventButton(eventButton)
,m_ButtonStates(buttonStates)
, m_Modifiers(modifiers)
{
}

mitk::MouseButtons mitk::MouseReleaseEvent::GetEventButton() const
{
  return m_EventButton;
}

void mitk::MouseReleaseEvent::SetEventButton(MouseButtons buttons)
{
  m_EventButton = buttons;
}

bool mitk::MouseReleaseEvent::IsSuperClassOf(InteractionEvent::Pointer baseClass)
{
  return dynamic_cast<MouseReleaseEvent*>(baseClass.GetPointer()) != NULL;
}

mitk::ModifierKeys mitk::MouseReleaseEvent::GetModifiers() const
{
  return m_Modifiers;
}

mitk::MouseButtons mitk::MouseReleaseEvent::GetButtonStates() const
{
  return m_ButtonStates;
}

void mitk::MouseReleaseEvent::SetModifiers(ModifierKeys modifiers)
{
  m_Modifiers = modifiers;
}

void mitk::MouseReleaseEvent::SetButtonStates(MouseButtons buttons)
{
  m_ButtonStates = buttons;
}

mitk::MouseReleaseEvent::~MouseReleaseEvent()
{
}

bool mitk::MouseReleaseEvent::MatchesTemplate(mitk::InteractionEvent::Pointer interactionEvent)
{
  mitk::MouseReleaseEvent* mre = dynamic_cast<mitk::MouseReleaseEvent*>(interactionEvent.GetPointer());
  if (mre == NULL)
  {
    return false;
  }
  return (this->GetEventButton() == mre->GetEventButton() && this->GetModifiers() == mre->GetModifiers()
      && this->GetButtonStates() == mre->GetButtonStates()

  );
}
