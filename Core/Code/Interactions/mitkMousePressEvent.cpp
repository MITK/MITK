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

mitk::MousePressEvent::MousePressEvent(mitk::BaseRenderer* baseRenderer, mitk::Point2D mousePosition, unsigned int buttonStates,
    unsigned int modifiers, unsigned int eventButton) :
    InteractionPositionEvent(baseRenderer, mousePosition, buttonStates, modifiers, "MousePressEvent")
{
  m_EventButton = eventButton;
}

mitk::MousePressEvent::~MousePressEvent()
{

}

bool mitk::MousePressEvent::isEqual(mitk::InteractionEvent* interactionEvent) {
  return (interactionEvent==interactionEvent);
}

