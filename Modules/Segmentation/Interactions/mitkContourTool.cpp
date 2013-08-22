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
  CONNECT_ACTION( 91, OnChangeActiveLabel );
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

bool mitk::ContourTool::OnChangeActiveLabel (Action* action, const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  if ( FeedbackContourTool::CanHandleEvent(stateEvent) < 1.0 ) return false;

  DataNode* workingNode( m_ToolManager->GetWorkingData(0) );
  assert (workingNode);

  mitk::LabelSetImage* lsImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());

  int value = lsImage->GetPixelValueByWorldCoordinate( positionEvent->GetWorldPosition() );

  lsImage->SetActiveLabel(value, true);

  return true;
}

bool mitk::ContourTool::OnMousePressed (Action* action, const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  if ( FeedbackContourTool::CanHandleEvent(stateEvent) < 1.0 ) return false;

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  DataNode* workingNode( m_ToolManager->GetWorkingData(0) );
  assert (workingNode);

  ContourModel* feedbackContour = this->GetFeedbackContour();
  assert (feedbackContour);
  feedbackContour->Initialize();

  int timestep = positionEvent->GetSender()->GetTimeStep();
  feedbackContour->Expand(timestep+1);

  feedbackContour->Close(timestep);
  feedbackContour->AddVertex( positionEvent->GetWorldPosition(), timestep );

  FeedbackContourTool::SetFeedbackContourVisible(true);
  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}

bool mitk::ContourTool::OnMouseMoved (Action* action, const StateEvent* stateEvent)
{
  if ( FeedbackContourTool::CanHandleEvent(stateEvent) < 1.0 ) return false;

  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  int timestep = positionEvent->GetSender()->GetTimeStep();

  ContourModel* feedbackContour = this->GetFeedbackContour();
  assert( feedbackContour );
  feedbackContour->AddVertex( positionEvent->GetWorldPosition(), timestep );

  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}

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
    assert(workingNode);

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

    ContourModel* feedbackContour = this->GetFeedbackContour();
    assert( feedbackContour);
    ContourModel::Pointer projectedContour = this->ProjectContourTo2DSlice( slice, feedbackContour );

    if (projectedContour.IsNull()) return false;

    this->FillContourInSlice( projectedContour, slice, image->GetLabelSet(), m_PaintingPixelValue );

    this->WriteBackSegmentationResult(positionEvent, slice);

    // 4. Make sure the result is drawn again --> is visible then.
    assert( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}
