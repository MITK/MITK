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

mitk::MouseReleaseEvent::MouseReleaseEvent(mitk::BaseRenderer* baseRenderer, mitk::Point2D mousePosition, mitk::EButtons buttonStates = BN_NoButton,
    mitk::EModifiers modifiers = MOD_NoModifiers, mitk::EButtons eventButton = BN_NoButton) :
    InteractionPositionEvent(baseRenderer, mousePosition, buttonStates, modifiers, "MouseReleaseEvent"),m_EventButton(eventButton)
{
}


mitk::EButtons mitk::MouseReleaseEvent::GetEventButton()
{
  return m_EventButton;
}

mitk::MouseReleaseEvent::~MouseReleaseEvent()
{

}

bool mitk::MouseReleaseEvent::isEqual(mitk::InteractionEvent::Pointer interactionEvent)
{
  mitk::MouseReleaseEvent* mpe = dynamic_cast< mitk::MouseReleaseEvent* >(interactionEvent.GetPointer());
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

