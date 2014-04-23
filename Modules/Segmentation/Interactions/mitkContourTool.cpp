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

#include "mitkToolManager.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
#include "mitkLabelSetImage.h"
#include "mitkContourUtils.h"
//#include "mitkSurfaceInterpolationController.h"
#include "mitkStateMachineAction.h"
#include "mitkInteractionEvent.h"
#include "mitkPositionEvent.h"

mitk::ContourTool::ContourTool()
:FeedbackContourTool("PressMoveReleaseWithCTRLInversion"),
 m_PaintingPixelValue(0),
 m_LogicInverted(false)
{
}

mitk::ContourTool::~ContourTool()
{
}

void mitk::ContourTool::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION( "PrimaryButtonPressed", OnMousePressed);
  CONNECT_FUNCTION( "Move", OnMouseMoved);
  CONNECT_FUNCTION( "Release", OnMouseReleased);
}

void mitk::ContourTool::Activated()
{
  m_LogicInverted = false;
  Superclass::Activated();
}

void mitk::ContourTool::Deactivated()
{
  Superclass::Deactivated();
}

bool mitk::ContourTool::OnMousePressed (StateMachineAction*, InteractionEvent* interactionEvent)
{
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  //const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(interactionEvent->GetEvent());
  if (!positionEvent) return false;

  if ( FeedbackContourTool::CanHandleEvent(interactionEvent) < 1.0 ) return false;

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  ContourModel* feedbackContour = mitk::FeedbackContourTool::GetFeedbackContour();
  assert (feedbackContour);
  feedbackContour->Initialize();

  int timestep = positionEvent->GetSender()->GetTimeStep();
  feedbackContour->Expand(timestep+1);

  feedbackContour->Close(timestep);
  Point3D pointIn3D = positionEvent->GetPositionInWorld();
  feedbackContour->AddVertex( pointIn3D, timestep );

  FeedbackContourTool::SetFeedbackContourVisible(true);
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}

bool mitk::ContourTool::OnMouseMoved( StateMachineAction*, InteractionEvent* interactionEvent )
{
  if ( FeedbackContourTool::CanHandleEvent(interactionEvent) < 1.0 ) return false;

  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  //const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  int timestep = positionEvent->GetSender()->GetTimeStep();

  ContourModel* feedbackContour = mitk::FeedbackContourTool::GetFeedbackContour();
  assert( feedbackContour );
  mitk::Point3D pointInWorld = positionEvent->GetPositionInWorld();
  feedbackContour->AddVertex( pointInWorld, timestep );

  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}

bool mitk::ContourTool::OnMouseReleased( StateMachineAction*, InteractionEvent* interactionEvent )
{
  if ( FeedbackContourTool::CanHandleEvent(interactionEvent) < 1.0 ) return false;

  // 1. Hide the feedback contour, find out which slice the user clicked, find out which slice of
  // the toolmanager's working image corresponds to that
  FeedbackContourTool::SetFeedbackContourVisible(false);

  InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  if (!positionEvent) return false;

  int timestep = positionEvent->GetSender()->GetTimeStep();

  LabelSetImage* workingImage = dynamic_cast<LabelSetImage*>(m_WorkingNode->GetData());
  assert(workingImage);

  const PlaneGeometry* planeGeometry = dynamic_cast<const PlaneGeometry*> (positionEvent->GetSender()->GetCurrentWorldGeometry2D() );
  if ( !planeGeometry ) return false;

  // 2. Slice is known, now we try to get it as a 2D image and project the contour into index coordinates of this slice
  Image::Pointer slice = SegTool2D::GetAffectedImageSliceAs2DImage( planeGeometry, workingImage, timestep );
  if ( slice.IsNull() )
  {
    MITK_ERROR << "Unable to extract slice.";
    return false;
  }

  // 3. Get our feedback contour
  ContourModel* feedbackContour = mitk::FeedbackContourTool::GetFeedbackContour();
  assert(feedbackContour);

  // 4. Project it into the extracted plane
  ContourModel::Pointer projectedContour = ContourModel::New();
  const mitk::Geometry3D* sliceGeometry = slice->GetGeometry();
  ContourUtils::ProjectContourTo2DSlice( sliceGeometry, feedbackContour, projectedContour );

  // 5. Transfer contour to working slice taking into account whether neighboring labels are locked or editable
  ContourUtils::FillContourInSlice( projectedContour, slice, m_PaintingPixelValue );

  // 6. Write back the slice into our working image
  SegTool2D::WriteBackSegmentationResult(positionEvent, slice);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  return true;
}
