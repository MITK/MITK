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

mitk::MouseMoveEvent::MouseMoveEvent(mitk::BaseRenderer* baseRenderer, mitk::Point2D mousePosition, mitk::EButtonStates buttonStates,
    mitk::EButtonStates modifiers) :
    InteractionPositionEvent(baseRenderer, mousePosition, buttonStates, modifiers, "MouseMoveEvent")
{
}

mitk::MouseMoveEvent::~MouseMoveEvent()
{

}

bool mitk::MouseMoveEvent::isEqual(mitk::InteractionEvent::Pointer interactionEvent)
{
  mitk::MouseMoveEvent* mpe = dynamic_cast< mitk::MouseMoveEvent* >(interactionEvent.GetPointer());
  if (mpe == NULL)
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

