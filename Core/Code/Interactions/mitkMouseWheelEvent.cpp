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

mitk::MouseWheelEvent::MouseWheelEvent(BaseRenderer* baseRenderer, Point2D mousePosition, MouseButtons buttonStates =
    NoButton, ModifierKeys modifiers = NoKey, int wheelDelta = 0) :
    InteractionPositionEvent(baseRenderer, mousePosition, buttonStates, modifiers, "MouseWheelEvent"), m_WheelDelta(wheelDelta)
{
}

int mitk::MouseWheelEvent::GetWheelDelta() const
{
  return m_WheelDelta;
}

mitk::MouseWheelEvent::~MouseWheelEvent()
{
}

bool mitk::MouseWheelEvent::isEqual(mitk::InteractionEvent::Pointer interactionEvent)
{
  const mitk::MouseWheelEvent* mwe = dynamic_cast<const MouseWheelEvent*>(interactionEvent.GetPointer());
  if (mwe == NULL)
  {
    return false;
  }
  return ((this->GetWheelDelta() * mwe->GetWheelDelta() > 0) // Consider WheelEvents to be equal if the scrolling is done in the same direction.
  && this->GetModifiers() == mwe->GetModifiers()
  && this->GetButtonStates() == mwe->GetButtonStates());
}

