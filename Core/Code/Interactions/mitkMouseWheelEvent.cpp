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

mitk::MouseWheelEvent::MouseWheelEvent(mitk::BaseRenderer* baseRenderer, mitk::Point2D mousePosition, mitk::MouseButtons buttonStates = NoButton,
    mitk::ModifierKeys modifiers = NoKey, int wheelDelta=0) :
    InteractionPositionEvent(baseRenderer, mousePosition, buttonStates, modifiers, "MouseWheelEvent"),m_WheelDelta(wheelDelta)
{
}

int mitk::MouseWheelEvent::GetWheelDelta()
{
  return m_WheelDelta;
}

mitk::MouseWheelEvent::~MouseWheelEvent()
{

}

bool mitk::MouseWheelEvent::isEqual(mitk::InteractionEvent::Pointer interactionEvent)
{
  mitk::MouseWheelEvent* mpe = dynamic_cast< mitk::MouseWheelEvent* >(interactionEvent.GetPointer());
  if (mpe == NULL)
  {
    return false;
  }
  // Consider WheelEvents to be equal if the scrolling is done in the same direction.
  if ((this->GetWheelDelta() * mpe->GetWheelDelta())>0)
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

