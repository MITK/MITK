/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkContourTool.h"

mitk::ContourTool::ContourTool(int paintingPixelValue)
  : FeedbackContourTool("PressMoveReleaseWithCTRLInversion"),
    m_PaintingPixelValue(paintingPixelValue)
{
}

mitk::ContourTool::~ContourTool()
{
}

void mitk::ContourTool::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("PrimaryButtonPressed", OnMousePressed);
  CONNECT_FUNCTION("Move", OnMouseMoved);
  CONNECT_FUNCTION("Release", OnMouseReleased);
  CONNECT_FUNCTION("InvertLogic", OnInvertLogic);
}

void mitk::ContourTool::Activated()
{
  Superclass::Activated();
}

void mitk::ContourTool::Deactivated()
{
  Superclass::Deactivated();
}

/**
 Just show the contour, insert the first point.
*/
void mitk::ContourTool::OnMousePressed(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
  if (!positionEvent)
    return;

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  this->ClearsCurrentFeedbackContour(true);
  mitk::Point3D point = positionEvent->GetPositionInWorld();
  this->AddVertexToCurrentFeedbackContour(point);

  FeedbackContourTool::SetFeedbackContourVisible(true);
  assert(positionEvent->GetSender()->GetRenderWindow());
  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
}

/**
 Insert the point to the feedback contour.
*/
void mitk::ContourTool::OnMouseMoved(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
  if (!positionEvent)
    return;

  mitk::Point3D point = positionEvent->GetPositionInWorld();
  this->AddVertexToCurrentFeedbackContour(point);

  assert(positionEvent->GetSender()->GetRenderWindow());
  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
}

/**
  Close the contour, project it to the image slice and fill it in 2D.
*/
void mitk::ContourTool::OnMouseReleased(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
  if (!positionEvent)
    return;

  assert(positionEvent->GetSender()->GetRenderWindow());
  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());

  this->WriteBackFeedbackContourAsSegmentationResult(positionEvent, m_PaintingPixelValue);
}

/**
  Called when the CTRL key is pressed. Will change the painting pixel value from 0 to 1 or from 1 to 0.
*/
void mitk::ContourTool::OnInvertLogic(StateMachineAction *, InteractionEvent *)
{
  // Inversion only for 0 and 1 as painting values
  if (m_PaintingPixelValue == 1)
  {
    m_PaintingPixelValue = 0;
    FeedbackContourTool::SetFeedbackContourColor(1.0, 0.0, 0.0);
  }
  else if (m_PaintingPixelValue == 0)
  {
    m_PaintingPixelValue = 1;
    FeedbackContourTool::SetFeedbackContourColorDefault();
  }
}
