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
    const mitk::Point2D mousePosition,
    const std::string eventClass)
: InteractionEvent(baseRenderer, eventClass)
, m_PointerPosition(mousePosition)
{
  if (GetSender() != NULL)
  {
    m_WorldPosition = GetSender()->Map2DRendererPositionTo3DWorldPosition(&m_PointerPosition);
  }
  else
  {
    m_WorldPosition.Fill(0);
  }
}

const mitk::Point2D mitk::InteractionPositionEvent::GetPointerPositionOnScreen()
{
  return m_PointerPosition;
}

const mitk::Point3D mitk::InteractionPositionEvent::GetPositionInWorld()
{
  return m_WorldPosition;
}

bool mitk::InteractionPositionEvent::MatchesTemplate(InteractionEvent::Pointer)
{
  return true;
}

mitk::InteractionPositionEvent::~InteractionPositionEvent()
{
}
