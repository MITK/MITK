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

#include "mitkContourTool.h"

#include "mitkToolManager.h"
#include "mitkOverwriteSliceImageFilter.h"

#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
//#include "mitkProperties.h"

mitk::ContourTool::ContourTool(int paintingPixelValue)
:FeedbackContourTool("PressMoveReleaseWithCTRLInversion"),
 m_PaintingPixelValue(paintingPixelValue)
{
}

mitk::ContourTool::~ContourTool()
{
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
bool mitk::ContourTool::OnMousePressed (Action* action, const StateEvent* stateEvent)
{
  if (!FeedbackContourTool::OnMousePressed( action, stateEvent )) return false;

  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

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
bool mitk::ContourTool::OnMouseMoved   (Action* action, const StateEvent* stateEvent)
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

/**
  Close the contour, project it to the image slice and fill it in 2D.
*/
bool mitk::ContourTool::OnMouseReleased(Action* action, const StateEvent* stateEvent)
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
  if ( SegTool2D::DetermineAffectedImageSlice( image, planeGeometry, affectedDimension, affectedSlice ) )
  {
    // 2. Slice is known, now we try to get it as a 2D image and project the contour into index coordinates of this slice
    Image::Pointer slice = SegTool2D::GetAffectedImageSliceAs2DImage( positionEvent, image );

    if ( slice.IsNull() )
    {
      MITK_ERROR << "Unable to extract slice." << std::endl;
      return false;
    }

    /*
    DataTreeNode::Pointer debugNode = DataTreeNode::New();
    debugNode->SetData( slice );
    debugNode->SetProperty( "name", StringProperty::New("extracted slice") );
    debugNode->SetProperty( "color", ColorProperty::New(1.0, 0.0, 0.0) );
    debugNode->SetProperty( "layer", FloatProperty::New(100) );
    m_ToolManager->GetDataStorage()->Add( debugNode );
    */

    Contour* feedbackContour( FeedbackContourTool::GetFeedbackContour() );
    Contour::Pointer projectedContour = FeedbackContourTool::ProjectContourTo2DSlice( slice, feedbackContour, true, false ); // true: actually no idea why this is neccessary, but it works :-(
                                                                                                                   // false: don't constrain the contour to the image's inside
    /*
    Contour::Pointer back = FeedbackContourTool::BackProjectContourFrom2DSlice( slice, projectedContour, true ); // true: actually no idea why this is neccessary, but it works :-(
    for (unsigned int idx = 0; idx < back->GetNumberOfPoints(); ++idx)
    {
      Point3D before = feedbackContour->GetPoints()->ElementAt(idx);
      Point3D inbtw = projectedContour->GetPoints()->ElementAt(idx);
      Point3D after = back->GetPoints()->ElementAt(idx);

      MITK_DEBUG << "before " << before << " zwischen " << inbtw << " after " << after;
    }
  */

    if (projectedContour.IsNull()) return false;

    FeedbackContourTool::FillContourInSlice( projectedContour, slice, m_PaintingPixelValue );

    // 5. Write the modified 2D working data slice back into the image
    OverwriteSliceImageFilter::Pointer slicewriter = OverwriteSliceImageFilter::New();
    slicewriter->SetInput( image );
    slicewriter->SetCreateUndoInformation( true );
    slicewriter->SetSliceImage( slice );
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

/**
  Called when the CTRL key is pressed. Will change the painting pixel value from 0 to 1 or from 1 to 0. 
*/
bool mitk::ContourTool::OnInvertLogic(Action* action, const StateEvent* stateEvent)
{
  if (!FeedbackContourTool::OnInvertLogic(action, stateEvent)) return false;

  // Inversion only for 0 and 1 as painting values
  if (m_PaintingPixelValue == 1)
  {
    m_PaintingPixelValue = 0;
    FeedbackContourTool::SetFeedbackContourColor( 1.0, 0.0, 0.0 );
  }
  else if (m_PaintingPixelValue == 0)
  {
    m_PaintingPixelValue = 1;
    FeedbackContourTool::SetFeedbackContourColorDefault();
  }

  return true;
}

