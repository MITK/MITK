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

#include "mitkInteractionPositionEvent.h"
#include "mitkException.h"
#include <string>



mitk::InteractionPositionEvent::InteractionPositionEvent(mitk::BaseRenderer* baseRenderer, mitk::Point2D mousePosition, unsigned int buttonStates, unsigned int modifiers, std::string eventClass)
: InteractionEvent(baseRenderer, eventClass),
  m_MousePosition(mousePosition),
  m_ButtonStates(buttonStates),
  m_Modifiers(modifiers)
{
  //TODO: map mouse position to global position
  //m_WorldPosition = ...
}

const mitk::Point2D* mitk::InteractionPositionEvent::GetMousePosition()
{
  return &m_MousePosition;
}

const mitk::Point3D* mitk::InteractionPositionEvent::GetWorldPosition()
{
  return &m_WorldPosition;
}

unsigned int mitk::InteractionPositionEvent::GetModifiers()
{
  return m_Modifiers;
}

unsigned int mitk::InteractionPositionEvent::GetButtonStates()
{
  return m_ButtonStates;
}

mitk::InteractionPositionEvent::~InteractionPositionEvent()
{
}

