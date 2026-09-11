/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkContourTool.h>

#include <mitkRenderingManager.h>

mitk::ContourTool::ContourTool(int paintingPixelValue)
  : FeedbackContourTool("PressMoveReleaseWithCTRLInversion"),
    m_PaintingPixelValue(paintingPixelValue),
    m_InitialPaintingPixelValue(paintingPixelValue)
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

  // State machine state and painting value outlive a deactivation. A tool left
  // inverted, e.g. by switching tools while CTRL is held, would otherwise come
  // back inverted while its cursor, pushed anew on activation, would not.
  this->ResetToStartState();
  this->SetPaintingPixelValue(m_InitialPaintingPixelValue);
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
  auto renderWindow = positionEvent->GetSender()->GetRenderWindow();
  assert(renderWindow);
  auto renderManager = mitk::RenderingManager::GetInstance();
  //This ensures that contour tools also make the renderwindow they interact with the focused
  //window. For this tools it does not happen automatically as the mouse press event does not
  //reach the RenderWindowManager. By Setting the focus it is possible to pass events like
  //hot keys for time or slice cycling to the window
  renderManager->SetRenderWindowFocus(renderWindow);
  renderManager->RequestUpdate(renderWindow);
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


  auto workingSeg = this->GetWorkingData();
  if (!workingSeg)
    return;
  auto activeLabel = workingSeg->GetActiveLabel();
  if (nullptr == activeLabel)
    return;
  const auto activeLabelValue = activeLabel->GetValue();

  this->WriteBackFeedbackContourAsSegmentationResult(positionEvent, activeLabelValue, m_PaintingPixelValue!=0);
}

/**
  Called when the CTRL key is pressed. Will change the painting pixel value from 0 to 1 or from 1 to 0.
*/
void mitk::ContourTool::OnInvertLogic(StateMachineAction *, InteractionEvent *)
{
  // Inversion only for 0 and 1 as painting values
  if (m_PaintingPixelValue == 1)
  {
    this->SetPaintingPixelValue(0);
  }
  else if (m_PaintingPixelValue == 0)
  {
    this->SetPaintingPixelValue(1);
  }

  // The inversion may be triggered by a key event, which unlike a mouse event
  // does not cause a render update on its own. The feedback contour is 2D only.
  mitk::RenderingManager::GetInstance()->RequestUpdateAll(mitk::RenderingManager::REQUEST_UPDATE_2DWINDOWS);
}

void mitk::ContourTool::SetPaintingPixelValue(int paintingPixelValue)
{
  m_PaintingPixelValue = paintingPixelValue;

  if (0 == paintingPixelValue)
  {
    FeedbackContourTool::SetFeedbackContourColor(1.0, 0.0, 0.0);
  }
  else
  {
    FeedbackContourTool::SetFeedbackContourColorDefault();
  }
}
