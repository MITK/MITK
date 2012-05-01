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

#include "mitkSetRegionTool.h"

#include "mitkToolManager.h"
#include "mitkOverwriteSliceImageFilter.h"

#include "ipSegmentation.h"

#include "mitkBaseRenderer.h"
#include "mitkImageDataItem.h"

#include "mitkOverwriteDirectedPlaneImageFilter.h"

mitk::SetRegionTool::SetRegionTool(int paintingPixelValue)
:FeedbackContourTool("PressMoveReleaseWithCTRLInversion"),
 m_PaintingPixelValue(paintingPixelValue),
 m_FillContour(false),
 m_StatusFillWholeSlice(false)
{
}

mitk::SetRegionTool::~SetRegionTool()
{
}

void mitk::SetRegionTool::Activated()
{
  Superclass::Activated();
}

void mitk::SetRegionTool::Deactivated()
{
  Superclass::Deactivated();
}

bool mitk::SetRegionTool::OnMousePressed (Action* action, const StateEvent* stateEvent)
{
  if (!FeedbackContourTool::OnMousePressed( action, stateEvent )) return false;

  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  // 1. Get the working image
  Image::Pointer workingSlice   = FeedbackContourTool::GetAffectedWorkingSlice( positionEvent );
  if ( workingSlice.IsNull() ) return false; // can't do anything without the segmentation
 
  // if click was outside the image, don't continue
  const Geometry3D* sliceGeometry = workingSlice->GetGeometry();
  itk::Index<2> projectedPointIn2D;
  sliceGeometry->WorldToIndex( positionEvent->GetWorldPosition(), projectedPointIn2D );
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
/*
  unsigned int rowSize = originalPicSlice->n[0];
*/
  ipMITKSegmentationTYPE* data = static_cast<ipMITKSegmentationTYPE*>(originalPicSlice->data);

  if ( data[oneContourOffset] == 0 ) // initial seed 0
  {
    for ( ; oneContourOffset < size; ++oneContourOffset )
    {
      if ( data[oneContourOffset] > 0 ) break;
    }
  }
  else if ( data[oneContourOffset] == 1 ) // initial seed 1
  {
    unsigned int lastValidPixel = size-1; // initialization, will be changed lateron
    bool inSeg = true;    // inside segmentation?
    for ( ; oneContourOffset < size; ++oneContourOffset )
    {
      if ( ( data[oneContourOffset] == 0 ) && inSeg ) // pixel 0 and inside-flag set: this happens at the first pixel outside a filled region
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
    // copy point from float* to mitk::Contour 
    Contour::Pointer contourInImageIndexCoordinates = Contour::New();
    contourInImageIndexCoordinates->Initialize();
    Point3D newPoint;
    for (int index = 0; index < numberOfContourPoints; ++index)
    {
      newPoint[0] = contourPoints[ 2 * index + 0 ];
      newPoint[1] = contourPoints[ 2 * index + 1];
      newPoint[2] = 0;

      contourInImageIndexCoordinates->AddVertex( newPoint - 0.5 );
    }

    m_SegmentationContourInWorldCoordinates = FeedbackContourTool::BackProjectContourFrom2DSlice( workingSlice->GetGeometry(), contourInImageIndexCoordinates, true ); // true, correct the result from ipMITKSegmentationGetContour8N

    // 3. Show the contour
    FeedbackContourTool::SetFeedbackContour( *m_SegmentationContourInWorldCoordinates );
    
    FeedbackContourTool::SetFeedbackContourVisible(true);
    mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
  }

  // always generate a second contour, containing the whole image (used when CTRL is pressed)
  {
    // copy point from float* to mitk::Contour 
    Contour::Pointer contourInImageIndexCoordinates = Contour::New();
    contourInImageIndexCoordinates->Initialize();
    Point3D newPoint;
    newPoint[0] = 0; newPoint[1] = 0; newPoint[2] = 0.0;
    contourInImageIndexCoordinates->AddVertex( newPoint );
    newPoint[0] = originalPicSlice->n[0]; newPoint[1] = 0; newPoint[2] = 0.0;
    contourInImageIndexCoordinates->AddVertex( newPoint );
    newPoint[0] = originalPicSlice->n[0]; newPoint[1] = originalPicSlice->n[1]; newPoint[2] = 0.0;
    contourInImageIndexCoordinates->AddVertex( newPoint );
    newPoint[0] = 0; newPoint[1] = originalPicSlice->n[1]; newPoint[2] = 0.0;
    contourInImageIndexCoordinates->AddVertex( newPoint );

    m_WholeImageContourInWorldCoordinates = FeedbackContourTool::BackProjectContourFrom2DSlice( workingSlice->GetGeometry(), contourInImageIndexCoordinates, true ); // true, correct the result from ipMITKSegmentationGetContour8N

    // 3. Show the contour
    FeedbackContourTool::SetFeedbackContour( *m_SegmentationContourInWorldCoordinates );
    
    FeedbackContourTool::SetFeedbackContourVisible(true);
    mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
  }


  free(contourPoints);

  return true;
}

bool mitk::SetRegionTool::OnMouseReleased(Action* action, const StateEvent* stateEvent)
{
  // 1. Hide the feedback contour, find out which slice the user clicked, find out which slice of the toolmanager's working image corresponds to that
  FeedbackContourTool::SetFeedbackContourVisible(false);
  
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
  
  if (!m_FillContour && !m_StatusFillWholeSlice) return true;
  
  if (!FeedbackContourTool::OnMouseReleased( action, stateEvent )) return false;

  DataNode* workingNode( m_ToolManager->GetWorkingData(0) );
  if (!workingNode) return false;

  Image* image = dynamic_cast<Image*>(workingNode->GetData());
  const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (positionEvent->GetSender()->GetCurrentWorldGeometry2D() ) );
  if ( !image || !planeGeometry ) return false;

  Image::Pointer slice = FeedbackContourTool::GetAffectedImageSliceAs2DImage( positionEvent, image );

  if ( slice.IsNull() )
  {
      MITK_ERROR << "Unable to extract slice." << std::endl;
      return false;
  }

  Contour* feedbackContour( FeedbackContourTool::GetFeedbackContour() );
  Contour::Pointer projectedContour = FeedbackContourTool::ProjectContourTo2DSlice( slice, feedbackContour, false, false ); // false: don't add 0.5 (done by FillContourInSlice)
  // false: don't constrain the contour to the image's inside
  if (projectedContour.IsNull()) return false;

  FeedbackContourTool::FillContourInSlice( projectedContour, slice, m_PaintingPixelValue );

  this->WriteBackSegmentationResult(positionEvent, slice);

  // 6. Make sure the result is drawn again --> is visible then. 
  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());

  m_WholeImageContourInWorldCoordinates = NULL;
  m_SegmentationContourInWorldCoordinates = NULL;

  return true;
}

/**
  Called when the CTRL key is pressed. Will change the painting pixel value from 0 to 1 or from 1 to 0. 
*/
bool mitk::SetRegionTool::OnInvertLogic(Action* action, const StateEvent* stateEvent)
{
  if (!FeedbackContourTool::OnInvertLogic(action, stateEvent)) return false;
  
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  if (m_StatusFillWholeSlice)
  {
    // use contour extracted from image data
    if (m_SegmentationContourInWorldCoordinates.IsNotNull())
      FeedbackContourTool::SetFeedbackContour( *m_SegmentationContourInWorldCoordinates );
    mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
  } 
  else
  {
    // use some artificial contour
    if (m_WholeImageContourInWorldCoordinates.IsNotNull())
      FeedbackContourTool::SetFeedbackContour( *m_WholeImageContourInWorldCoordinates );
    mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
  }

  m_StatusFillWholeSlice = !m_StatusFillWholeSlice;

  return true;
}

