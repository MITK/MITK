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

mitk::MousePressEvent::MousePressEvent(mitk::BaseRenderer* baseRenderer, mitk::Point2D mousePosition, mitk::EButtons buttonStates = BN_NoButton,
    mitk::EModifiers modifiers = MOD_NoModifiers, mitk::EButtons eventButton = BN_NoButton) :
    InteractionPositionEvent(baseRenderer, mousePosition, buttonStates, modifiers, "MousePressEvent"),m_EventButton(eventButton)
{
}

mitk::EButtons mitk::MousePressEvent::GetEventButton()
{
  return m_EventButton;
}

mitk::MousePressEvent::~MousePressEvent()
{

}

bool mitk::MousePressEvent::isEqual(mitk::InteractionEvent::Pointer interactionEvent)
{
  mitk::MousePressEvent* mpe = dynamic_cast< mitk::MousePressEvent* >(interactionEvent.GetPointer());
  if (mpe == NULL)
  {
    return false;
  }

  if (this->GetEventButton() != mpe->GetEventButton())
  {
    return false;
  }
  if (this->GetModifiers() != mpe->GetModifiers())
  {
    return false;
  }
  if (this->GetButtonStates() != mpe->GetButtonStates())
  {
    return false;
  }
  return true;

}

