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

mitk::Point3D mitk::InteractionPositionEvent::GetPlanePositionInWorld() const
{
  const PlaneGeometry* planeGeometry = GetSender()->GetCurrentWorldPlaneGeometry();
  mitk::Point2D position;
  mitk::Point3D worldPos;
  if (GetSender()->GetMapperID() == BaseRenderer::Standard2D) {
    worldPos = GetPositionInWorld();
  } else {
    GetSender()->PickWorldPoint(m_PointerPosition, worldPos, BaseRenderer::PickingMode::WorldPointPicking);
  }
  planeGeometry->Map(worldPos, position);
  mitk::Point3D point;
  planeGeometry->Map(position, point);

  return point;
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
