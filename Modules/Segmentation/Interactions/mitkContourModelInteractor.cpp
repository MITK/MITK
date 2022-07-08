/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkContourModelInteractor.h"

#include "mitkToolManager.h"

#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"

#include <mitkInteractionPositionEvent.h>

#include <mitkInteractionConst.h>

mitk::ContourModelInteractor::ContourModelInteractor()
{
}

void mitk::ContourModelInteractor::ConnectActionsAndFunctions()
{
  CONNECT_CONDITION("checkisOverPoint", OnCheckPointClick);
  CONNECT_CONDITION("mouseMove", IsHovering);

  CONNECT_FUNCTION("movePoint", OnMovePoint);
  CONNECT_FUNCTION("deletePoint", OnDeletePoint);
  CONNECT_FUNCTION("finish", OnFinishEditing);
}

mitk::ContourModelInteractor::~ContourModelInteractor()
{
}

void mitk::ContourModelInteractor::SetRestrictedArea(mitk::ContourModel* restrictedArea)
{
  m_RestrictedArea = restrictedArea;
}

bool mitk::ContourModelInteractor::OnCheckPointClick(const InteractionEvent *interactionEvent)
{
  const auto *positionEvent =
    dynamic_cast<const mitk::InteractionPositionEvent *>(interactionEvent);
  if (!positionEvent)
    return false;

  const auto timeStep = interactionEvent->GetSender()->GetTimeStep(GetDataNode()->GetData());

  auto *contour = dynamic_cast<mitk::ContourModel *>(this->GetDataNode()->GetData());
  if (contour == nullptr)
  {
    MITK_ERROR << "Invalid Contour";
    return false;
  }

  contour->Deselect();

  mitk::Point3D click = positionEvent->GetPositionInWorld();

  bool isVertexSelected = contour->SelectControlVertexAt(click, ContourModelInteractor::eps, timeStep);

  if (isVertexSelected)
  {
    auto foundVertex = contour->GetSelectedVertex();
    const auto restrictedVs = m_RestrictedArea->GetVertexList(timeStep);
    for (auto restrictedV : restrictedVs)
    {
      if (restrictedV->Coordinates == foundVertex->Coordinates)
      {
        isVertexSelected = false;
        contour->Deselect();
        break;
      }
    }
  }

  if (isVertexSelected)
  {
    mitk::RenderingManager::GetInstance()->RequestUpdate(interactionEvent->GetSender()->GetRenderWindow());
  }

  return isVertexSelected;
}

void mitk::ContourModelInteractor::OnDeletePoint(StateMachineAction *, InteractionEvent *)
{
  auto *contour = dynamic_cast<mitk::ContourModel *>(this->GetDataNode()->GetData());
  contour->RemoveVertex(contour->GetSelectedVertex());
}

bool mitk::ContourModelInteractor::IsHovering(const InteractionEvent *interactionEvent)
{
  const auto *positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
  if (!positionEvent)
    return false;

  const auto timeStep = interactionEvent->GetSender()->GetTimeStep(GetDataNode()->GetData());

  auto *contour = dynamic_cast<mitk::ContourModel *>(this->GetDataNode()->GetData());

  mitk::Point3D currentPosition = positionEvent->GetPositionInWorld();

  bool isHover = false;
  this->GetDataNode()->GetBoolProperty("contour.hovering", isHover, positionEvent->GetSender());
  if (contour->IsNearContour(currentPosition, ContourModelInteractor::eps, timeStep))
  {
    if (isHover == false)
    {
      this->GetDataNode()->SetBoolProperty("contour.hovering", true);
      mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
    }
  }
  else
  {
    if (isHover == true)
    {
      this->GetDataNode()->SetBoolProperty("contour.hovering", false);
      mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
    }
  }
  this->m_lastMousePosition = currentPosition;

  return false;
}

void mitk::ContourModelInteractor::OnMovePoint(StateMachineAction *, InteractionEvent *interactionEvent)
{
  const auto *positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
  if (!positionEvent)
    return;

  auto *contour = dynamic_cast<mitk::ContourModel *>(this->GetDataNode()->GetData());

  mitk::Vector3D translation;
  mitk::Point3D currentPosition = positionEvent->GetPositionInWorld();
  translation[0] = currentPosition[0] - this->m_lastMousePosition[0];
  translation[1] = currentPosition[1] - this->m_lastMousePosition[1];
  translation[2] = currentPosition[2] - this->m_lastMousePosition[2];
  contour->ShiftSelectedVertex(translation);

  this->m_lastMousePosition = positionEvent->GetPositionInWorld();
  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
}

void mitk::ContourModelInteractor::OnMoveContour(StateMachineAction *, InteractionEvent *interactionEvent)
{
  const auto *positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
  if (!positionEvent)
    return;

  const auto timeStep = interactionEvent->GetSender()->GetTimeStep(GetDataNode()->GetData());

  auto *contour = dynamic_cast<mitk::ContourModel *>(this->GetDataNode()->GetData());
  mitk::Vector3D translation;
  mitk::Point3D currentPosition = positionEvent->GetPositionInWorld();
  translation[0] = currentPosition[0] - this->m_lastMousePosition[0];
  translation[1] = currentPosition[1] - this->m_lastMousePosition[1];
  translation[2] = currentPosition[2] - this->m_lastMousePosition[2];
  contour->ShiftContour(translation, timeStep);

  this->m_lastMousePosition = positionEvent->GetPositionInWorld();

  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
}

void mitk::ContourModelInteractor::OnFinishEditing(StateMachineAction *, InteractionEvent *)
{
}
