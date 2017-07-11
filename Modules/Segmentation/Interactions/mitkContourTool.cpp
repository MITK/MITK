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

#include "mitkContourTool.h"

#include "mitkAbstractTransformGeometry.h"
#include "mitkOverwriteDirectedPlaneImageFilter.h"
#include "mitkOverwriteSliceImageFilter.h"
#include "mitkToolManager.h"

#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
//#include "mitkProperties.h"
//#include "mitkPlanarCircle.h"
#include "mitkLabelSetImage.h"

#include "mitkInteractionEvent.h"
#include "mitkStateMachineAction.h"

mitk::ContourTool::ContourTool(int paintingPixelValue)
  : FeedbackContourTool("PressMoveReleaseWithCTRLInversion"),
    m_PaintingPixelValue(paintingPixelValue),
    m_CurrentLabelID(1)
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
  mitk::InteractionPositionEvent *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
  if (!positionEvent)
    return;

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  int timestep = positionEvent->GetSender()->GetTimeStep();

  ContourModel *contour = FeedbackContourTool::GetFeedbackContour();
  // Clear feedback contour
  contour->Initialize();
  // expand time bounds because our contour was initialized
  contour->Expand(timestep + 1);
  // draw as a closed contour
  contour->SetClosed(true, timestep);
  // add first mouse position
  mitk::Point3D point = positionEvent->GetPositionInWorld();
  contour->AddVertex(point, timestep);

  FeedbackContourTool::SetFeedbackContourVisible(true);
  assert(positionEvent->GetSender()->GetRenderWindow());
  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
}

/**
 Insert the point to the feedback contour.
*/
void mitk::ContourTool::OnMouseMoved(StateMachineAction *, InteractionEvent *interactionEvent)
{
  mitk::InteractionPositionEvent *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
  if (!positionEvent)
    return;

  int timestep = positionEvent->GetSender()->GetTimeStep();

  ContourModel *contour = FeedbackContourTool::GetFeedbackContour();
  mitk::Point3D point = positionEvent->GetPositionInWorld();
  contour->AddVertex(point, timestep);

  assert(positionEvent->GetSender()->GetRenderWindow());
  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
}

/**
  Close the contour, project it to the image slice and fill it in 2D.
*/
void mitk::ContourTool::OnMouseReleased(StateMachineAction *, InteractionEvent *interactionEvent)
{
  // 1. Hide the feedback contour, find out which slice the user clicked, find out which slice of the toolmanager's
  // working image corresponds to that
  FeedbackContourTool::SetFeedbackContourVisible(false);

  mitk::InteractionPositionEvent *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
  // const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent)
    return;

  assert(positionEvent->GetSender()->GetRenderWindow());
  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());

  DataNode *workingNode(m_ToolManager->GetWorkingData(0));
  if (!workingNode)
    return;

  Image *image = dynamic_cast<Image *>(workingNode->GetData());
  const PlaneGeometry *planeGeometry((positionEvent->GetSender()->GetCurrentWorldPlaneGeometry()));
  if (!image || !planeGeometry)
    return;

  // Check if it is a multilabel-image
  // If yes, get the new drawing color from it.
  // Otherwise nothing happens.
  mitk::LabelSetImage *labelSetImage = dynamic_cast<LabelSetImage *>(image);
  if (labelSetImage)
  {
    mitk::Label *label = labelSetImage->GetActiveLabel(labelSetImage->GetActiveLayer());
    m_CurrentLabelID = label->GetValue();
  }
  else
  {
    m_CurrentLabelID = 1;
  }

  const AbstractTransformGeometry *abstractTransformGeometry(
    dynamic_cast<const AbstractTransformGeometry *>(positionEvent->GetSender()->GetCurrentWorldPlaneGeometry()));
  if (!image || abstractTransformGeometry)
    return;

  // 2. Slice is known, now we try to get it as a 2D image and project the contour into index coordinates of this slice
  Image::Pointer slice = SegTool2D::GetAffectedImageSliceAs2DImage(positionEvent, image);

  if (slice.IsNull())
  {
    MITK_ERROR << "Unable to extract slice." << std::endl;
    return;
  }

  ContourModel *feedbackContour = FeedbackContourTool::GetFeedbackContour();
  ContourModel::Pointer projectedContour = FeedbackContourTool::ProjectContourTo2DSlice(
    slice, feedbackContour, true, false); // true: actually no idea why this is neccessary, but it works :-(

  if (projectedContour.IsNull())
    return;

  int timestep = positionEvent->GetSender()->GetTimeStep();

  // m_PaintingPixelValue only decides whether to paint or erase
  mitk::ContourModelUtils::FillContourInSlice(
    projectedContour, timestep, slice, image, (m_PaintingPixelValue * m_CurrentLabelID));

  // this->WriteBackSegmentationResult(positionEvent, slice);
  SegTool2D::WriteBackSegmentationResult(positionEvent, slice);

  // 4. Make sure the result is drawn again --> is visible then.
  assert(positionEvent->GetSender()->GetRenderWindow());
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
