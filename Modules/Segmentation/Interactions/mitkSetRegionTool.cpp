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

#include "mitkSetRegionTool.h"

#include "mitkToolManager.h"
#include "mitkImageCast.h"
#include "mitkBaseRenderer.h"
#include "mitkImageDataItem.h"
#include "mitkLabelSetImage.h"
#include "mitkLegacyAdaptors.h"
#include "mitkContourUtils.h"

#include "ipSegmentation.h"

mitk::SetRegionTool::SetRegionTool()
:FeedbackContourTool("PressMoveReleaseWithCTRLInversion"),
 m_FillContour(false),
 m_StatusFillWholeSlice(false)
{
}

mitk::SetRegionTool::~SetRegionTool()
{
}

void mitk::SetRegionTool::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION( "PrimaryButtonPressed", OnMousePressed);
  CONNECT_FUNCTION( "Release", OnMouseReleased);
  CONNECT_FUNCTION( "InvertLogic", OnInvertLogic);
}

void mitk::SetRegionTool::Activated()
{
  Superclass::Activated();
}

void mitk::SetRegionTool::Deactivated()
{
  Superclass::Deactivated();
}

bool mitk::SetRegionTool::OnChangeActiveLabel (StateMachineAction*, InteractionEvent* interactionEvent)
{
  if ( FeedbackContourTool::CanHandleEvent(stateEvent) < 1.0 ) return false;

  DataNode* workingNode( m_ToolManager->GetWorkingData(0) );
  assert(workingNode);

  LabelSetImage* workingImage = dynamic_cast<LabelSetImage*>(workingNode->GetData());
  assert(workingImage);

  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  int timestep = positionEvent->GetSender()->GetTimeStep();

  int pixelValue = workingImage->GetPixelValueByWorldCoordinate( positionEvent->GetWorldPosition(), timestep );

  workingImage->SetActiveLabel(pixelValue);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  return true;
}

bool mitk::SetRegionTool::OnMousePressed ( StateMachineAction*, InteractionEvent* interactionEvent )
{
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  //const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  int timestep = positionEvent->GetSender()->GetTimeStep();

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  if ( FeedbackContourTool::CanHandleEvent(interactionEvent) < 1.0 ) return false;

  // 1. Get the working image
  Image::Pointer workingSlice = FeedbackContourTool::GetAffectedWorkingSlice( positionEvent );
  if ( workingSlice.IsNull() ) return false; // can't do anything without the segmentation

  // if click was outside the image, don't continue
  const Geometry3D* sliceGeometry = workingSlice->GetGeometry();
  itk::Index<2> projectedPointIn2D;
  sliceGeometry->WorldToIndex( positionEvent->GetPositionInWorld(), projectedPointIn2D );
  if ( !sliceGeometry->IsIndexInside( projectedPointIn2D ) )
  {
    MITK_ERROR << "point apparently not inside segmentation slice" << std::endl;
    return false; // can't use that as a seed point
  }

  // Convert to ipMITKSegmentationTYPE (because ipMITKSegmentationGetContour8N relys on that data type)
  itk::Image< ipMITKSegmentationTYPE, 2 >::Pointer correctPixelTypeImage;
  CastToItkImage( workingSlice, correctPixelTypeImage );
  assert (correctPixelTypeImage.IsNotNull() );

  // possible bug in CastToItkImage ?
  // direction maxtrix is wrong/broken/not working after CastToItkImage, leading to a failed assertion in
  // mitk/Core/DataStructures/mitkSlicedGeometry3D.cpp, 479:
  // virtual void mitk::SlicedGeometry3D::SetSpacing(const mitk::Vector3D&): Assertion `aSpacing[0]>0 && aSpacing[1]>0 && aSpacing[2]>0' failed
  // solution here: we overwrite it with an unity matrix
  itk::Image< ipMITKSegmentationTYPE, 2 >::DirectionType imageDirection;
  imageDirection.SetIdentity();
  correctPixelTypeImage->SetDirection(imageDirection);

  Image::Pointer temporarySlice = Image::New();
  //  temporarySlice = ImportItkImage( correctPixelTypeImage );
  CastToMitkImage( correctPixelTypeImage, temporarySlice );

  // check index positions
  mitkIpPicDescriptor* originalPicSlice = mitkIpPicNew();
  CastToIpPicDescriptor( temporarySlice, originalPicSlice );

  int m_SeedPointMemoryOffset = projectedPointIn2D[1] * originalPicSlice->n[0] + projectedPointIn2D[0];

  if ( m_SeedPointMemoryOffset >= static_cast<int>( originalPicSlice->n[0] * originalPicSlice->n[1] ) ||
       m_SeedPointMemoryOffset < 0 )
  {
    MITK_ERROR << "Memory offset calculation if mitk::SetRegionTool has some serious flaw! Aborting.." << std::endl;
    return false;
  }

  // 2. Determine the contour that surronds the selected "piece of the image"

  // find a contour seed point
  unsigned int oneContourOffset = static_cast<unsigned int>( m_SeedPointMemoryOffset ); // safe because of earlier check if m_SeedPointMemoryOffset < 0

  /**
    * The logic of finding a starting point for the contour is the following:
    *
    *  - If the initial seed point is 0, we are either inside a hole or outside of every segmentation.
    *    We move to the right until we hit a 1, which must be part of a contour.
    *
    *  - If the initial seed point is 1, then ...
    *    we now do the same (running to the right) until we hit a 1
    *
    *  In both cases the found contour point is used to extract a contour and
    *  then a test is applied to find out if the initial seed point is contained
    *  in the contour. If this is the case, filling should be applied, otherwise
    *  nothing is done.
    */
  unsigned int size = originalPicSlice->n[0] * originalPicSlice->n[1];

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(m_ToolManager->GetWorkingData(0)->GetData());
  int activeLayer = workingImage->GetActiveLayer();
  int activePixelValue = workingImage->GetActiveLabel(activeLayer)->GetIndex();

/*
  unsigned int rowSize = originalPicSlice->n[0];
*/
  ipMITKSegmentationTYPE* data = static_cast<ipMITKSegmentationTYPE*>(originalPicSlice->data);

  if ( data[oneContourOffset] != activePixelValue ) // ) // initial seed 0
  {
    for ( ; oneContourOffset < size; ++oneContourOffset )
    {
      if ( data[oneContourOffset] == activePixelValue ) break;
    }
  }
  else if ( data[oneContourOffset] == activePixelValue ) // initial seed 1
  {
    unsigned int lastValidPixel = size-1; // initialization, will be changed lateron
    bool inSeg = true;    // inside segmentation?
    for ( ; oneContourOffset < size; ++oneContourOffset )
    {
      if ( (data[oneContourOffset] != activePixelValue) && inSeg ) // pixel 0 and inside-flag set: this happens at the first pixel outside a filled region
      {
        inSeg = false;
        lastValidPixel = oneContourOffset - 1; // store the last pixel position inside a filled region
        break;
      }
      else // pixel 1, inside-flag doesn't matter: this happens while we are inside a filled region
      {
        inSeg = true; // first iteration lands here
      }
    }
    oneContourOffset = lastValidPixel;
  }
  else
  {
    MITK_ERROR << "Fill/Erase was never intended to work with other than binary images." << std::endl;
    m_FillContour = false;
    return false;
  }

  if (oneContourOffset == size) // nothing found until end of slice
  {
    m_FillContour = false;
    return false;
  }

  int numberOfContourPoints( 0 );
  int newBufferSize( 0 );
  //MITK_INFO << "getting contour from offset " << oneContourOffset << " ("<<oneContourOffset%originalPicSlice->n[0]<<","<<oneContourOffset/originalPicSlice->n[0]<<")"<<std::endl;
  float* contourPoints = ipMITKSegmentationGetContour8N( originalPicSlice, oneContourOffset, numberOfContourPoints, newBufferSize ); // memory allocated with malloc

  //MITK_INFO << "contourPoints " << contourPoints << " (N="<<numberOfContourPoints<<")"<<std::endl;
  assert(contourPoints == NULL || numberOfContourPoints > 0);

  bool cursorInsideContour = ipMITKSegmentationIsInsideContour( contourPoints, numberOfContourPoints, projectedPointIn2D[0], projectedPointIn2D[1]);

  // decide if contour should be filled or not
  m_FillContour = cursorInsideContour;

  if (m_FillContour)
  {
    // copy point from float* to mitk::ContourModel
    ContourModel::Pointer contourInImageIndexCoordinates = ContourModel::New();
//    contourInImageIndexCoordinates->Expand(timeStep + 1);
//    contourInImageIndexCoordinates->SetClosed(true, timeStep);
    Point3D newPoint;

    for (int index = 0; index < numberOfContourPoints; ++index)
    {
      newPoint[0] = contourPoints[ 2 * index + 0 ];
      newPoint[1] = contourPoints[ 2 * index + 1];
      newPoint[2] = 0;

      newPoint -= Point3D::VectorType(0.5);
      contourInImageIndexCoordinates->AddVertex(newPoint, timestep);
    }

    mitk::ContourModel* feedbackContour = FeedbackContourTool::GetFeedbackContour();
    assert( feedbackContour );
    ContourUtils::BackProjectContourFrom2DSlice( workingSlice->GetGeometry(), contourInImageIndexCoordinates, feedbackContour );
    feedbackContour->Modified();

    // Show the contour
    FeedbackContourTool::SetFeedbackContourVisible(true);
    mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
  }

/*
  // always generate a second contour, containing the whole image (used when CTRL is pressed)
  {
    // copy point from float* to mitk::Contour
    ContourModel::Pointer contourInImageIndexCoordinates = ContourModel::New();
    contourInImageIndexCoordinates->Expand(timeStep + 1);
    contourInImageIndexCoordinates->SetClosed(true, timeStep);
    Point3D newPoint;
    newPoint[0] = 0; newPoint[1] = 0; newPoint[2] = 0.0;
    contourInImageIndexCoordinates->AddVertex( newPoint, timestep);
    newPoint[0] = originalPicSlice->n[0]; newPoint[1] = 0; newPoint[2] = 0.0;
    contourInImageIndexCoordinates->AddVertex( newPoint, timestep);
    newPoint[0] = originalPicSlice->n[0]; newPoint[1] = originalPicSlice->n[1]; newPoint[2] = 0.0;
    contourInImageIndexCoordinates->AddVertex( newPoint, timestep);
    newPoint[0] = 0; newPoint[1] = originalPicSlice->n[1]; newPoint[2] = 0.0;
    contourInImageIndexCoordinates->AddVertex( newPoint, timestep);

//    m_WholeImageContourInWorldCoordinates = ContourUtils::BackProjectContourFrom2DSlice( workingSlice->GetGeometry(), contourInImageIndexCoordinates, timestep);
    ContourUtils::BackProjectContourFrom2DSlice( workingSlice->GetGeometry(), contourInImageIndexCoordinates, m_SegmentationContourInWorldCoordinates, timestep);

    // 3. Show the contour
    FeedbackContourTool::SetFeedbackContour( *m_SegmentationContourInWorldCoordinates );

    FeedbackContourTool::SetFeedbackContourVisible(true);
    mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
  }
*/
  free(contourPoints);

  return true;
}

bool mitk::SetRegionTool::OnMouseReleased( StateMachineAction*, InteractionEvent* interactionEvent )
{
  // 1. Hide the feedback contour, find out which slice the user clicked, find out which slice
  // of the toolmanager's working image corresponds to that
  FeedbackContourTool::SetFeedbackContourVisible(false);

  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  //const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  int timestep = positionEvent->GetSender()->GetTimeStep();

  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());

  if (!m_FillContour && !m_StatusFillWholeSlice) return true;

  if ( SegTool2D::CanHandleEvent(stateEvent) < 1.0 ) return false;

  DataNode* workingNode( m_ToolManager->GetWorkingData(0) );
  if (!workingNode) return false;

  LabelSetImage* image = dynamic_cast<LabelSetImage*>(workingNode->GetData());
  const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (positionEvent->GetSender()->GetCurrentWorldGeometry2D() ) );
  if ( !image || !planeGeometry ) return false;

  Image::Pointer slice = SegTool2D::GetAffectedImageSliceAs2DImage( positionEvent, image );

  if ( slice.IsNull() )
  {
    MITK_ERROR << "Unable to extract slice." << std::endl;
    return false;
  }

  ContourModel* feedbackContour = FeedbackContourTool::GetFeedbackContour();
  assert(feedbackContour);
  ContourModel::Pointer projectedContour = ContourModel::New();
  const mitk::Geometry3D* sliceGeometry = slice->GetGeometry();
  ContourUtils::ProjectContourTo2DSlice( sliceGeometry, feedbackContour, projectedContour );

  // stamp contour in 3D ooords into working slice
  ContourUtils::FillContourInSlice( projectedContour, slice, m_PaintingPixelValue );
  // write working slice back into working volume
  this->WriteBackSegmentationResult(positionEvent, slice);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  return true;
}
