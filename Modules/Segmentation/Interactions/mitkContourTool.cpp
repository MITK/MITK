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
#include "mitkOverwriteSliceImageFilter.h"
#include "mitkOverwriteDirectedPlaneImageFilter.h"

#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
#include "mitkLabelSetImage.h"


mitk::ContourTool::ContourTool()
:FeedbackContourTool("PressMoveReleaseWithCTRLInversion"),
 m_PaintingPixelValue(0),
 m_LogicInverted(false)
{
  // great magic numbers
  CONNECT_ACTION( 80, OnMousePressed );
  CONNECT_ACTION( 90, OnMouseMoved );
  CONNECT_ACTION( 42, OnMouseReleased );
}

mitk::ContourTool::~ContourTool()
{
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

/**
 Just show the contour, insert the first point.
*/
bool mitk::ContourTool::OnMousePressed (Action* action, const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  if ( FeedbackContourTool::CanHandleEvent(stateEvent) < 1.0 ) return false;

  DataNode* workingNode( m_ToolManager->GetWorkingData(0) );
  if (!workingNode) return false;

  Contour* contour = FeedbackContourTool::GetFeedbackContour();
  contour->Initialize();
  contour->AddVertex( positionEvent->GetWorldPosition() );

  FeedbackContourTool::SetFeedbackContourVisible(true);
  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}

/**
 Insert the point to the feedback contour.
*/
bool mitk::ContourTool::OnMouseMoved (Action* action, const StateEvent* stateEvent)
{
  if ( FeedbackContourTool::CanHandleEvent(stateEvent) < 1.0 ) return false;

  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  Contour* contour = FeedbackContourTool::GetFeedbackContour();
  contour->AddVertex( positionEvent->GetWorldPosition() );

  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}

/**
  Close the contour, project it to the image slice and fill it in 2D.
*/
bool mitk::ContourTool::OnMouseReleased (Action* action, const StateEvent* stateEvent)
{
    // 1. Hide the feedback contour, find out which slice the user clicked, find out which slice of the toolmanager's working image corresponds to that
    FeedbackContourTool::SetFeedbackContourVisible(false);

    const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
    if (!positionEvent) return false;

    assert( positionEvent->GetSender()->GetRenderWindow() );
    mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

    if ( FeedbackContourTool::CanHandleEvent(stateEvent) < 1.0 ) return false;

    DataNode* workingNode( m_ToolManager->GetWorkingData(0) );
    if (!workingNode) return false;

    LabelSetImage* image = dynamic_cast<LabelSetImage*>(workingNode->GetData());
    const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (positionEvent->GetSender()->GetCurrentWorldGeometry2D() ) );
    if ( !image || !planeGeometry ) return false;

    // 2. Slice is known, now we try to get it as a 2D image and project the contour into index coordinates of this slice
    Image::Pointer slice = SegTool2D::GetAffectedImageSliceAs2DImage( positionEvent, image );

    if ( slice.IsNull() )
    {
      MITK_ERROR << "Unable to extract slice." << std::endl;
      return false;
    }

    Contour* feedbackContour( FeedbackContourTool::GetFeedbackContour() );
    Contour::Pointer projectedContour = mitk::FeedbackContourTool::ProjectContourTo2DSlice( slice, feedbackContour, false, true );

    if (projectedContour.IsNull()) return false;

    mitk::FeedbackContourTool::FillContourInSlice( projectedContour, slice, image->GetLabelSet(), m_PaintingPixelValue );

    this->WriteBackSegmentationResult(positionEvent, slice);

    // 4. Make sure the result is drawn again --> is visible then.
    assert( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}
