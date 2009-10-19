/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkCorrectorTool2D.h"
#include "mitkCorrectorAlgorithm.h"

#include "mitkToolManager.h"
#include "mitkOverwriteSliceImageFilter.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"

#include "mitkCorrectorTool2D.xpm"

namespace mitk {
  MITK_TOOL_MACRO(MITKEXT_CORE_EXPORT, CorrectorTool2D, "Correction tool");
}

mitk::CorrectorTool2D::CorrectorTool2D(int paintingPixelValue)
:FeedbackContourTool("PressMoveRelease"),
 m_PaintingPixelValue(paintingPixelValue)
{
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
  if (!FeedbackContourTool::OnMousePressed( action, stateEvent )) return false;

  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  Contour* contour = FeedbackContourTool::GetFeedbackContour();
  contour->Initialize();
  contour->AddVertex( positionEvent->GetWorldPosition() );
  
  FeedbackContourTool::SetFeedbackContourVisible(true);

  return true;
}

bool mitk::CorrectorTool2D::OnMouseMoved   (Action* action, const StateEvent* stateEvent)
{
  if (!FeedbackContourTool::OnMouseMoved( action, stateEvent )) return false;

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
  
  if (!FeedbackContourTool::OnMouseReleased( action, stateEvent )) return false;

  DataTreeNode* workingNode( m_ToolManager->GetWorkingData(0) );
  if (!workingNode) return false;

  Image* image = dynamic_cast<Image*>(workingNode->GetData());
  const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (positionEvent->GetSender()->GetCurrentWorldGeometry2D() ) );
  if ( !image || !planeGeometry ) return false;

  int affectedDimension( -1 );
  int affectedSlice( -1 );
  if ( FeedbackContourTool::DetermineAffectedImageSlice( image, planeGeometry, affectedDimension, affectedSlice ) )
  {
    // 2. Slice is known, now we try to get it as a 2D image and project the contour into index coordinates of this slice
    m_WorkingSlice = FeedbackContourTool::GetAffectedImageSliceAs2DImage( positionEvent, image );

    if ( m_WorkingSlice.IsNull() )
    {
      LOG_ERROR << "Unable to extract slice." << std::endl;
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
      LOG_ERROR << "Caught exception '" << e.what() << "'" << std::endl;
    }


    // 5. Write the modified 2D working data slice back into the image
    OverwriteSliceImageFilter::Pointer slicewriter = OverwriteSliceImageFilter::New();
    slicewriter->SetInput( image );
    slicewriter->SetCreateUndoInformation( true );
    slicewriter->SetSliceImage( algorithm->GetOutput() );
    slicewriter->SetSliceDimension( affectedDimension );
    slicewriter->SetSliceIndex( affectedSlice );
    slicewriter->SetTimeStep( positionEvent->GetSender()->GetTimeStep( image ) );
    slicewriter->Update();

    // 6. Make sure the result is drawn again --> is visible then. 
    assert( positionEvent->GetSender()->GetRenderWindow() );
    mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );
  }
  else
  {
    InteractiveSegmentationBugMessage( "FeedbackContourTool could not determine which slice of the image you are drawing on." );
  }

  return true;
}
    
