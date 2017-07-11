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

  CONNECT_FUNCTION("movePoints", OnMovePoint);
  CONNECT_FUNCTION("deletePoint", OnDeletePoint);
  CONNECT_FUNCTION("finish", OnFinishEditing);
}

mitk::ContourModelInteractor::~ContourModelInteractor()
{
}

bool mitk::ContourModelInteractor::OnCheckPointClick(const InteractionEvent *interactionEvent)
{
  const InteractionPositionEvent *positionEvent =
    dynamic_cast<const mitk::InteractionPositionEvent *>(interactionEvent);
  if (!positionEvent)
    return false;

  int timestep = positionEvent->GetSender()->GetTimeStep();

  mitk::ContourModel *contour = dynamic_cast<mitk::ContourModel *>(this->GetDataNode()->GetData());

  contour->Deselect();

  mitk::Point3D click = positionEvent->GetPositionInWorld();

  if (contour->SelectVertexAt(click, 1.5, timestep))
  {
    contour->SetSelectedVertexAsControlPoint();
    mitk::RenderingManager::GetInstance()->RequestUpdate(interactionEvent->GetSender()->GetRenderWindow());
    m_lastMousePosition = click;

    mitk::Geometry3D *contourGeometry = dynamic_cast<Geometry3D *>(contour->GetGeometry(timestep));

    if (contourGeometry->IsInside(click))
    {
      m_lastMousePosition = click;
      return true;
    }
    else
      return false;
  }
  else
  {
    return false;
  }
  return true;
}

void mitk::ContourModelInteractor::OnDeletePoint(StateMachineAction *, InteractionEvent *)
{
  mitk::ContourModel *contour = dynamic_cast<mitk::ContourModel *>(this->GetDataNode()->GetData());
  contour->RemoveVertex(contour->GetSelectedVertex());
}

bool mitk::ContourModelInteractor::IsHovering(const InteractionEvent *interactionEvent)
{
  const InteractionPositionEvent *positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
  if (!positionEvent)
    return false;

  int timestep = positionEvent->GetSender()->GetTimeStep();

  mitk::ContourModel *contour = dynamic_cast<mitk::ContourModel *>(this->GetDataNode()->GetData());

  mitk::Point3D currentPosition = positionEvent->GetPositionInWorld();

  bool isHover = false;
  this->GetDataNode()->GetBoolProperty("contour.hovering", isHover, positionEvent->GetSender());
  if (contour->IsNearContour(currentPosition, 1.5, timestep))
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
  return false;
}

void mitk::ContourModelInteractor::OnMovePoint(StateMachineAction *, InteractionEvent *interactionEvent)
{
  const InteractionPositionEvent *positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
  if (!positionEvent)
    return;

  mitk::ContourModel *contour = dynamic_cast<mitk::ContourModel *>(this->GetDataNode()->GetData());

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
  const InteractionPositionEvent *positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
  if (!positionEvent)
    return;

  int timestep = positionEvent->GetSender()->GetTimeStep();

  mitk::ContourModel *contour = dynamic_cast<mitk::ContourModel *>(this->GetDataNode()->GetData());
  mitk::Vector3D translation;
  mitk::Point3D currentPosition = positionEvent->GetPositionInWorld();
  translation[0] = currentPosition[0] - this->m_lastMousePosition[0];
  translation[1] = currentPosition[1] - this->m_lastMousePosition[1];
  translation[2] = currentPosition[2] - this->m_lastMousePosition[2];
  contour->ShiftContour(translation, timestep);

  this->m_lastMousePosition = positionEvent->GetPositionInWorld();

  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
}

void mitk::ContourModelInteractor::OnFinishEditing(StateMachineAction *, InteractionEvent *interactionEvent)
{
  mitk::ContourModel *contour = dynamic_cast<mitk::ContourModel *>(this->GetDataNode()->GetData());
  contour->Deselect();
  mitk::RenderingManager::GetInstance()->RequestUpdate(interactionEvent->GetSender()->GetRenderWindow());
}
