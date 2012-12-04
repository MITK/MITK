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
#include <string>

mitk::InteractionPositionEvent::InteractionPositionEvent(mitk::BaseRenderer* baseRenderer,
    mitk::Point2D mousePosition,
    mitk::MouseButtons buttonStates,
    mitk::ModifierKeys modifiers,
    std::string eventClass) :
    InteractionEvent(baseRenderer, eventClass), m_MousePosition(mousePosition), m_ButtonStates(buttonStates), m_Modifiers(modifiers)
{
  if (GetSender() != NULL)
  {
    m_WorldPosition = GetSender()->GetWorldPosition(m_MousePosition);
  }
  else
  {
    m_WorldPosition[0] = 0;
    m_WorldPosition[1] = 0;
    m_WorldPosition[2] = 0;
    MITK_WARN<< "Renderer not initialized. WorldCoordinates bogus!";
  }

}

const mitk::Point2D* mitk::InteractionPositionEvent::GetMousePosition()
{
  return &m_MousePosition;
}

const mitk::Point3D* mitk::InteractionPositionEvent::GetWorldPosition()
{
  return &m_WorldPosition;
}

mitk::ModifierKeys mitk::InteractionPositionEvent::GetModifiers()
{
  return m_Modifiers;
}

mitk::MouseButtons mitk::InteractionPositionEvent::GetButtonStates()
{
  return m_ButtonStates;
}

bool mitk::InteractionPositionEvent::isEqual(InteractionEvent::Pointer)
{
  return true;
}

mitk::InteractionPositionEvent::~InteractionPositionEvent()
{
}

