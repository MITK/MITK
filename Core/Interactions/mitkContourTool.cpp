#include "mitkContourTool.h"

#include "mitkToolManager.h"
#include "mitkOverwriteSliceImageFilter.h"

mitk::ContourTool::ContourTool(int paintingPixelValue)
:SegTool2D("PressMoveReleaseWithCTRLInversion"),
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
  if (!SegTool2D::OnMousePressed( action, stateEvent )) return false;

  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  Contour* contour = SegTool2D::GetFeedbackContour();
  contour->Initialize();
  contour->AddVertex( positionEvent->GetWorldPosition() );
  
  SegTool2D::SetFeedbackContourVisible(true);

  return true;
}

/**
 Insert the point to the feedback contour.
*/
bool mitk::ContourTool::OnMouseMoved   (Action* action, const StateEvent* stateEvent)
{
  if (!SegTool2D::OnMouseMoved( action, stateEvent )) return false;

  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  Contour* contour = SegTool2D::GetFeedbackContour();
  contour->AddVertex( positionEvent->GetWorldPosition() );

  assert( positionEvent->GetSender()->GetRenderWindow() );
  positionEvent->GetSender()->GetRenderWindow()->RequestUpdate();

  return true;
}

/**
  Close the contour, project it to the image slice and fill it in 2D.
*/
bool mitk::ContourTool::OnMouseReleased(Action* action, const StateEvent* stateEvent)
{
  // 1. Hide the feedback contour, find out which slice the user clicked, find out which slice of the toolmanager's working image corresponds to that
  SegTool2D::SetFeedbackContourVisible(false);
  
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  assert( positionEvent->GetSender()->GetRenderWindow() );
  positionEvent->GetSender()->GetRenderWindow()->RequestUpdate();
  
  if (!SegTool2D::OnMouseReleased( action, stateEvent )) return false;

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
      std::cerr << "Unable to extract slice." << std::endl;
      return false;
    }

    Contour* feedbackContour( SegTool2D::GetFeedbackContour() );
    Contour::Pointer projectedContour = SegTool2D::ProjectContourTo2DSlice( slice, feedbackContour, true, false ); // true: actually no idea why this is neccessary, but it works :-(
                                                                                                                   // false: don't constrain the contour to the image's inside
    if (projectedContour.IsNull()) return false;

    SegTool2D::FillContourInSlice( projectedContour, slice, m_PaintingPixelValue );

    // 5. Write the modified 2D working data slice back into the image
    OverwriteSliceImageFilter::Pointer slicewriter = OverwriteSliceImageFilter::New();
    slicewriter->SetInput( image );
    slicewriter->SetCreateUndoInformation( true );
    slicewriter->SetSliceImage( slice );
    slicewriter->SetSliceDimension( affectedDimension );
    slicewriter->SetSliceIndex( affectedSlice );
    slicewriter->Update();

    // 6. Make sure the result is drawn again --> is visible then. 
    assert( positionEvent->GetSender()->GetRenderWindow() );
    positionEvent->GetSender()->GetRenderWindow()->RequestUpdate();
  }
  else
  {
    SliceBasedSegmentationBugMessage( "SegTool2D could not determine which slice of the image you are drawing on." );
  }

  return true;
}

/**
  Called when the CTRL key is pressed. Will change the painting pixel value from 0 to 1 or from 1 to 0. 
*/
bool mitk::ContourTool::OnInvertLogic(Action* action, const StateEvent* stateEvent)
{
  if (!SegTool2D::OnInvertLogic(action, stateEvent)) return false;

  // Inversion only for 0 and 1 as painting values
  if (m_PaintingPixelValue == 1)
  {
    m_PaintingPixelValue = 0;
    SegTool2D::SetFeedbackContourColor( 1.0, 0.0, 0.0 );
  }
  else if (m_PaintingPixelValue == 0)
  {
    m_PaintingPixelValue = 1;
    SegTool2D::SetFeedbackContourColorDefault();
  }

  return true;
}

