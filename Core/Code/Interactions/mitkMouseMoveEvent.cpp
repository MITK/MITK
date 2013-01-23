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
#include "mitkMouseMoveEvent.h"

mitk::MouseMoveEvent::MouseMoveEvent(mitk::BaseRenderer* baseRenderer, mitk::Point2D mousePosition = NULL, mitk::MouseButtons buttonStates =
    NoButton, mitk::ModifierKeys modifiers = NoKey) :
    InteractionPositionEvent(baseRenderer, mousePosition,  "MouseMoveEvent"),m_ButtonStates(buttonStates),m_Modifiers(modifiers)
{
}

bool mitk::MouseMoveEvent::IsSuperClassOf(InteractionEvent::Pointer baseClass)
{
  MouseMoveEvent* event = dynamic_cast<MouseMoveEvent*>(baseClass.GetPointer());
  if (event != NULL)
  {
    return true;
  }
  return false;
}

mitk::ModifierKeys mitk::MouseMoveEvent::GetModifiers() const
{
  return m_Modifiers;
}

mitk::MouseButtons mitk::MouseMoveEvent::GetButtonStates() const
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

bool mitk::MouseMoveEvent::isEqual(mitk::InteractionEvent::Pointer interactionEvent)
{
  mitk::MouseMoveEvent* mpe = dynamic_cast<mitk::MouseMoveEvent*>(interactionEvent.GetPointer());
  if (mpe == NULL)
  {
    return false;
  }
  return (this->GetModifiers() == mpe->GetModifiers() && this->GetButtonStates() == mpe->GetButtonStates());
}

