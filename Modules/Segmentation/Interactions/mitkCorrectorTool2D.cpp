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

#include "mitkCorrectorTool2D.h"
#include "mitkCorrectorAlgorithm.h"

#include "mitkToolManager.h"
#include "mitkOverwriteSliceImageFilter.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"

#include "mitkCorrectorTool2D.xpm"

#include "mitkOverwriteDirectedPlaneImageFilter.h"

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, CorrectorTool2D, "Correction tool");
}

mitk::CorrectorTool2D::CorrectorTool2D(int paintingPixelValue)
:FeedbackContourTool("PressMoveRelease"),
 m_PaintingPixelValue(paintingPixelValue)
{
  // great magic numbers
  CONNECT_ACTION( 80, OnMousePressed );
  CONNECT_ACTION( 90, OnMouseMoved );
  CONNECT_ACTION( 42, OnMouseReleased );

  GetFeedbackContour()->SetClosed( false ); // don't close the contour to a polygon
}

mitk::CorrectorTool2D::~CorrectorTool2D()
{
}

const char** mitk::CorrectorTool2D::GetXPM() const
{
  return mitkCorrectorTool2D_xpm;
}

const char* mitk::CorrectorTool2D::GetName() const
{
  return "Correction";
}

void mitk::CorrectorTool2D::Activated()
{
  Superclass::Activated();
}

void mitk::CorrectorTool2D::Deactivated()
{
  Superclass::Deactivated();
}

bool mitk::CorrectorTool2D::OnMousePressed (Action* action, const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  if ( FeedbackContourTool::CanHandleEvent(stateEvent) < 1.0 ) return false;

  Contour* contour = FeedbackContourTool::GetFeedbackContour();
  contour->Initialize();
  contour->AddVertex( positionEvent->GetWorldPosition() );

  FeedbackContourTool::SetFeedbackContourVisible(true);

  return true;
}

bool mitk::CorrectorTool2D::OnMouseMoved   (Action* action, const StateEvent* stateEvent)
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

bool mitk::CorrectorTool2D::OnMouseReleased(Action* action, const StateEvent* stateEvent)
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

  Image* image = dynamic_cast<Image*>(workingNode->GetData());
  const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (positionEvent->GetSender()->GetCurrentWorldGeometry2D() ) );
  if ( !image || !planeGeometry ) return false;

  // 2. Slice is known, now we try to get it as a 2D image and project the contour into index coordinates of this slice
  m_WorkingSlice = FeedbackContourTool::GetAffectedImageSliceAs2DImage( positionEvent, image );

  if ( m_WorkingSlice.IsNull() )
  {
      MITK_ERROR << "Unable to extract slice." << std::endl;
      return false;
  }

  CorrectorAlgorithm::Pointer algorithm = CorrectorAlgorithm::New();
  algorithm->SetInput( m_WorkingSlice );
  algorithm->SetContour( FeedbackContourTool::GetFeedbackContour() );
  try
  {
      algorithm->UpdateLargestPossibleRegion();
  }
  catch ( std::exception& e )
  {
      MITK_ERROR << "Caught exception '" << e.what() << "'" << std::endl;
  }

  mitk::Image::Pointer resultSlice = mitk::Image::New();
  resultSlice->Initialize(algorithm->GetOutput());
  resultSlice->SetVolume(algorithm->GetOutput()->GetData());

  this->WriteBackSegmentationResult(positionEvent, resultSlice);

  return true;
}

