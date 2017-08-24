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

mitk::InteractionPositionEvent::InteractionPositionEvent(mitk::BaseRenderer *baseRenderer,
                                                         const mitk::Point2D &mousePosition)
  : InteractionEvent(baseRenderer), m_PointerPosition(mousePosition)
{
}

mitk::Point2D mitk::InteractionPositionEvent::GetPointerPositionOnScreen() const
{
  return m_PointerPosition;
}

mitk::Point3D mitk::InteractionPositionEvent::GetPositionInWorld() const
{
  Point3D worldPos;
  this->GetSender()->DisplayToWorld(m_PointerPosition, worldPos);
  return worldPos;
}

bool mitk::InteractionPositionEvent::IsEqual(const InteractionEvent &other) const
{
  return Superclass::IsEqual(other);
}

mitk::InteractionPositionEvent::~InteractionPositionEvent()
{
}

bool mitk::InteractionPositionEvent::IsSuperClassOf(const InteractionEvent::Pointer &baseClass) const
{
  return (dynamic_cast<InteractionPositionEvent *>(baseClass.GetPointer()) != nullptr);
}
