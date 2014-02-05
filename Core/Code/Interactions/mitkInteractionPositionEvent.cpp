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
    const mitk::Point2D& mousePosition)
: InteractionEvent(baseRenderer)
, m_PointerPosition(mousePosition)
{
  if (GetSender() != NULL)
  {
    m_WorldPosition = GetSender()->Map2DRendererPositionTo3DWorldPosition(m_PointerPosition);
  }
  else
  {
    m_WorldPosition.Fill(0);
  }
}

mitk::Point2D mitk::InteractionPositionEvent::GetPointerPositionOnScreen() const
{
  return m_PointerPosition;
}

mitk::Point3D mitk::InteractionPositionEvent::GetPositionInWorld() const
{
  return m_WorldPosition;
}

bool mitk::InteractionPositionEvent::IsEqual(const InteractionEvent& other) const
{
  return Superclass::IsEqual(other);
}

mitk::InteractionPositionEvent::~InteractionPositionEvent()
{
}

bool mitk::InteractionPositionEvent::IsSuperClassOf(const InteractionEvent::Pointer& baseClass) const
{
  return (dynamic_cast<InteractionPositionEvent*>(baseClass.GetPointer()) != NULL) ;
}
