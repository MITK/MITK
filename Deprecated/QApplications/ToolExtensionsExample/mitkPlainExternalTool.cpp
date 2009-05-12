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

#include "mitkPlainExternalTool.h"

#include "mitkToolManager.h"
#include "mitkOverwriteSliceImageFilter.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"

#include "mitkPlainExternalTool.xpm"

mitk::PlainExternalTool::PlainExternalTool()
:FeedbackContourTool("PressMoveRelease")
{
  GetFeedbackContour()->SetClosed( false ); // don't close the contour to a polygon
}

mitk::PlainExternalTool::~PlainExternalTool()
{
}

const char** mitk::PlainExternalTool::GetXPM() const
{
  return mitkPlainExternalTool_xpm;
}

const char* mitk::PlainExternalTool::GetName() const
{
  return "External";
}

const char* mitk::PlainExternalTool::GetGroup() const
{
  // actually not needed, "default" is the default group name
  return "default";
}


void mitk::PlainExternalTool::Activated()
{
  Superclass::Activated();
}

void mitk::PlainExternalTool::Deactivated()
{
  Superclass::Deactivated();
}

bool mitk::PlainExternalTool::OnMousePressed (Action* action, const StateEvent* stateEvent)
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

bool mitk::PlainExternalTool::OnMouseMoved   (Action* action, const StateEvent* stateEvent)
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

bool mitk::PlainExternalTool::OnMouseReleased(Action* action, const StateEvent* stateEvent)
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
    mitk::Image::Pointer workingSlice = FeedbackContourTool::GetAffectedImageSliceAs2DImage( positionEvent, image );

    if ( workingSlice.IsNull() )
    {
      std::cerr << "Unable to extract slice." << std::endl;
      return false;
    }

    // etc etc. Example will be extended later. Look at other tools in MITK to see what to do here.
    
/*
    // 5. Write the modified 2D working data slice back into the image
    OverwriteSliceImageFilter::Pointer slicewriter = OverwriteSliceImageFilter::New();
    slicewriter->SetInput( image );
    slicewriter->SetCreateUndoInformation( true );
    slicewriter->SetSliceImage( temporarySlice );
    slicewriter->SetSliceDimension( affectedDimension );
    slicewriter->SetSliceIndex( affectedSlice );
    slicewriter->SetTimeStep( positionEvent->GetSender()->GetTimeStep( image ) );
    slicewriter->Update();
*/

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
    
