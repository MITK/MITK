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

#include "mitkRegionGrowingTool.h"
#include "mitkToolManager.h"
#include "mitkOverwriteSliceImageFilter.h"

#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"

#include "ipSegmentation.h"

#include "mitkRegionGrowingTool.xpm"

#define ROUND(a)     ((a)>0 ? (int)((a)+0.5) : -(int)(0.5-(a)))

mitk::RegionGrowingTool::RegionGrowingTool()
:FeedbackContourTool("PressMoveRelease"),
 m_LowerThreshold(200),
 m_UpperThreshold(200),
 m_InitialLowerThreshold(200),
 m_InitialUpperThreshold(200),
 m_ScreenYPositionAtStart(0),
 m_OriginalPicSlice(NULL),
 m_SeedPointMemoryOffset(0),
 m_MouseDistanceScaleFactor(3.0),
 m_LastWorkingSeed(-1),
 m_FillFeedbackContour(true)
{
}

mitk::RegionGrowingTool::~RegionGrowingTool()
{
}

const char** mitk::RegionGrowingTool::GetXPM() const
{
  return mitkRegionGrowingTool_xpm;
}

const char* mitk::RegionGrowingTool::GetName() const
{
  return "Region growing";
}

void mitk::RegionGrowingTool::Activated()
{
  Superclass::Activated();
}

void mitk::RegionGrowingTool::Deactivated()
{
  Superclass::Deactivated();
}

/**
 1 Determine which slice is clicked into
 2 Determine if the user clicked inside or outside of the segmentation
 3 Depending on the pixel value under the mouse click position, two different things happen: (separated out into OnMousePressedInside and OnMousePressedOutside)
   3.1 Create a skeletonization of the segmentation and try to find a nice cut
     3.1.1 Call a ipSegmentation algorithm to create a nice cut
     3.1.2 Set the result of this algorithm as the feedback contour
   3.2 Initialize region growing
     3.2.1 Determine memory offset inside the original image
     3.2.2 Determine initial region growing parameters from the level window settings of the image
     3.2.3 Perform a region growing (which generates a new feedback contour)
 */
bool mitk::RegionGrowingTool::OnMousePressed (Action* action, const StateEvent* stateEvent)
{
  //ToolLogger::SetVerboseness(3);

  ToolLogger::Logger(1) << "OnMousePressed" << std::endl;
  if (FeedbackContourTool::OnMousePressed( action, stateEvent ))
  {
    ToolLogger::Logger(2) << "OnMousePressed: FeedbackContourTool says ok" << std::endl;

    // 1. Find out which slice the user clicked, find out which slice of the toolmanager's reference and working image corresponds to that
    const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
    if (positionEvent)
    {
      ToolLogger::Logger(2) << "OnMousePressed: got positionEvent" << std::endl;
      
      m_ReferenceSlice = FeedbackContourTool::GetAffectedReferenceSlice( positionEvent );
      m_WorkingSlice   = FeedbackContourTool::GetAffectedWorkingSlice( positionEvent );

      if ( m_WorkingSlice.IsNotNull() ) // can't do anything without the segmentation
      {
        ToolLogger::Logger(2) << "OnMousePressed: got working slice" << std::endl;

        // 2. Determine if the user clicked inside or outside of the segmentation
          const Geometry3D* workingSliceGeometry = m_WorkingSlice->GetGeometry();
          itk::Index<2> projectedPointInWorkingSlice2D;
          workingSliceGeometry->WorldToIndex( positionEvent->GetWorldPosition(), projectedPointInWorkingSlice2D);

          if ( workingSliceGeometry->IsIndexInside( projectedPointInWorkingSlice2D ) )
          {
            ToolLogger::Logger(3) << "OnMousePressed: point " << positionEvent->GetWorldPosition() << " (index coordinates " << projectedPointInWorkingSlice2D << ") IS in working slice" << std::endl;

            // Convert to ipMITKSegmentationTYPE (because getting pixels relys on that data type)
            itk::Image< ipMITKSegmentationTYPE, 2 >::Pointer correctPixelTypeImage;
            CastToItkImage( m_WorkingSlice, correctPixelTypeImage );
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

          ipPicDescriptor* workingPicSlice = temporarySlice->GetSliceData()->GetPicDescriptor();
         
          int initialWorkingOffset = projectedPointInWorkingSlice2D[1] * workingPicSlice->n[0] + projectedPointInWorkingSlice2D[0];

          if ( initialWorkingOffset < static_cast<int>( workingPicSlice->n[0] * workingPicSlice->n[1] ) &&
               initialWorkingOffset >= 0 )
          {
            // 3. determine the pixel value under the last click
            bool inside = static_cast<ipMITKSegmentationTYPE*>(workingPicSlice->data)[initialWorkingOffset] != 0;
            m_PaintingPixelValue = inside ? 0 : 1; // if inside, we want to remove a part, otherwise we want to add something

            if ( m_LastWorkingSeed >= static_cast<int>( workingPicSlice->n[0] * workingPicSlice->n[1] ) ||
                 m_LastWorkingSeed < 0 )
            {
              inside = false; 
            }
              
            if ( m_ReferenceSlice.IsNotNull() )
            {
              ToolLogger::Logger(2) << "OnMousePressed: got reference slice" << std::endl;

              m_OriginalPicSlice = m_ReferenceSlice->GetSliceData()->GetPicDescriptor();

              // 3.1. Switch depending on the pixel value
              if (inside)
              {
                OnMousePressedInside(action, stateEvent, workingPicSlice, initialWorkingOffset);
              }
              else
              {
                OnMousePressedOutside(action, stateEvent);
              }
            }
          }
        }
      }
    }
  }
  
  ToolLogger::Logger(2) << "end OnMousePressed" << std::endl;
  return true;
}

/**
 3.1 Create a skeletonization of the segmentation and try to find a nice cut
   3.1.1 Call a ipSegmentation algorithm to create a nice cut
   3.1.2 Set the result of this algorithm as the feedback contour
*/ 
bool mitk::RegionGrowingTool::OnMousePressedInside(Action* itkNotUsed( action ), const StateEvent* stateEvent, ipPicDescriptor* workingPicSlice, int initialWorkingOffset)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent()); // checked in OnMousePressed
  // 3.1.1. Create a skeletonization of the segmentation and try to find a nice cut
  // apply the skeletonization-and-cut algorithm
  // generate contour to remove
  // set m_ReferenceSlice = NULL so nothing will happen during mouse move
  // remember to fill the contour with 0 in mouserelease
  ipPicDescriptor* segmentationHistory = ipMITKSegmentationCreateGrowerHistory( workingPicSlice, m_LastWorkingSeed, NULL ); // free again
  if (segmentationHistory)
  {
    tCutResult cutContour = ipMITKSegmentationGetCutPoints( workingPicSlice, segmentationHistory, initialWorkingOffset ); // tCutResult is a ipSegmentation type
    ipPicFree( segmentationHistory );
    if (cutContour.cutIt) 
    {
      // 3.1.2 copy point from float* to mitk::Contour 
      Contour::Pointer contourInImageIndexCoordinates = Contour::New();
      contourInImageIndexCoordinates->Initialize();
      Point3D newPoint;
      for (int index = 0; index < cutContour.deleteSize; ++index)
      {
        newPoint[0] = cutContour.deleteCurve[ 2 * index + 0 ];
        newPoint[1] = cutContour.deleteCurve[ 2 * index + 1];
        newPoint[2] = 0;

          contourInImageIndexCoordinates->AddVertex( newPoint );
      }

      free(cutContour.traceline);
      free(cutContour.deleteCurve); // perhaps visualize this for fun?
      free(cutContour.onGradient);

      Contour::Pointer contourInWorldCoordinates = FeedbackContourTool::BackProjectContourFrom2DSlice( m_WorkingSlice, contourInImageIndexCoordinates, true ); // true: sub 0.5 for ipSegmentation correction

      FeedbackContourTool::SetFeedbackContour( *contourInWorldCoordinates );
      FeedbackContourTool::SetFeedbackContourVisible(true);
      mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );
      m_FillFeedbackContour = true;
    }
    else
    {
      m_FillFeedbackContour = false;
    }

  }
  else
  {
    m_FillFeedbackContour = false;
  }

  m_ReferenceSlice = NULL;

  return true;
}

/**
 3.2 Initialize region growing
   3.2.1 Determine memory offset inside the original image
   3.2.2 Determine initial region growing parameters from the level window settings of the image
   3.2.3 Perform a region growing (which generates a new feedback contour)
*/
bool mitk::RegionGrowingTool::OnMousePressedOutside(Action* itkNotUsed( action ), const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent()); // checked in OnMousePressed
  // 3.2 If we have a reference image, then perform an initial region growing, considering the reference image's level window

  // if click was outside the image, don't continue
  const Geometry3D* sliceGeometry = m_ReferenceSlice->GetGeometry();
  itk::Index<2> projectedPointIn2D;
  sliceGeometry->WorldToIndex( positionEvent->GetWorldPosition(), projectedPointIn2D );

  if ( sliceGeometry->IsIndexInside( projectedPointIn2D ) )
  {
    ToolLogger::Logger(3) << "OnMousePressed: point " << positionEvent->GetWorldPosition() << " (index coordinates " << projectedPointIn2D << ") IS in reference slice" << std::endl;

    // 3.2.1 Remember Y cursor position and initial seed point
    m_ScreenYPositionAtStart = static_cast<int>(positionEvent->GetDisplayPosition()[1]);
   
    m_SeedPointMemoryOffset = projectedPointIn2D[1] * m_OriginalPicSlice->n[0] + projectedPointIn2D[0];
    m_LastWorkingSeed = m_SeedPointMemoryOffset; // remember for skeletonization

    if ( m_SeedPointMemoryOffset < static_cast<int>( m_OriginalPicSlice->n[0] * m_OriginalPicSlice->n[1] ) &&
         m_SeedPointMemoryOffset >= 0 )
    {

      // 3.2.2 Get level window from reference DataTreeNode
      //       Use some logic to determine initial gray value bounds
      LevelWindow lw(0, 500);
      m_ToolManager->GetReferenceData(0)->GetLevelWindow(lw); // will fill lw if levelwindow property is present, otherwise won't touch it.

      m_VisibleWindow = lw.GetWindow();

      static bool initializedAlready = false; // just evaluated once

      if (!initializedAlready)
      {
        m_InitialLowerThreshold = static_cast<int>(m_VisibleWindow / 10.0); // 20% of the visible gray values
        m_InitialUpperThreshold = static_cast<int>(m_VisibleWindow / 10.0);

        initializedAlready = true;
      }
      
      m_LowerThreshold = m_InitialLowerThreshold;
      m_UpperThreshold = m_InitialUpperThreshold;

      DisplayGeometry* displayGeometry = positionEvent->GetSender()->GetDisplayGeometry();
      if (displayGeometry)
      {
        m_MouseDistanceScaleFactor = m_VisibleWindow / ( 3.0 * displayGeometry->GetDisplayHeight() );
      }

      // 3.2.3. Actually perform region growing
      ipPicDescriptor* result = PerformRegionGrowingAndUpdateContour();
      ipMITKSegmentationFree( result);

      // display the contour
      FeedbackContourTool::SetFeedbackContourVisible(true);
      mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
        
      m_FillFeedbackContour = true;
    }
  }

  return true;
}

/**
 If in region growing mode (m_ReferenceSlice != NULL), then
 1. Calculate the new thresholds from mouse position (relative to first position)
 2. Perform a new region growing and update the feedback contour
*/
bool mitk::RegionGrowingTool::OnMouseMoved   (Action* action, const StateEvent* stateEvent)
{
  if (FeedbackContourTool::OnMouseMoved( action, stateEvent ))
  {
    if ( m_ReferenceSlice.IsNotNull() && m_OriginalPicSlice ) 
    {
      const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
      if (positionEvent) 
      {
        // 1. Calculate new region growing parameters

        float screenYDifference = positionEvent->GetDisplayPosition()[1] - m_ScreenYPositionAtStart;

        m_LowerThreshold = m_InitialLowerThreshold + static_cast<int>( screenYDifference * m_MouseDistanceScaleFactor );
        if (m_LowerThreshold < 0) m_LowerThreshold = 0;

        m_UpperThreshold = m_InitialUpperThreshold + static_cast<int>( screenYDifference * m_MouseDistanceScaleFactor );
        if (m_UpperThreshold < 0) m_UpperThreshold = 0;

        // std::cout << "new interval: l " << m_LowerThreshold << " u " << m_UpperThreshold << std::endl;
        
        // 2. Perform region growing again and show the result
        ipPicDescriptor* result = PerformRegionGrowingAndUpdateContour();
        ipMITKSegmentationFree( result );

        // 3. Update the contour
        mitk::RenderingManager::GetInstance()->ForceImmediateUpdate(positionEvent->GetSender()->GetRenderWindow());
      }
    }
  }

  return true;
}

/**
 If the feedback contour should be filled, then it is done here. (Contour is NOT filled, when skeletonization is done but no nice cut was found)
*/
bool mitk::RegionGrowingTool::OnMouseReleased(Action* action, const StateEvent* stateEvent)
{
  if (FeedbackContourTool::OnMouseReleased( action, stateEvent ))
  {
    // 1. If we have a working slice, use the contour to fill a new piece on segmentation on it (or erase a piece that was selected by ipMITKSegmentationGetCutPoints)
    if ( m_WorkingSlice.IsNotNull() && m_OriginalPicSlice )
    {
      const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
      if (positionEvent)
      {
        // remember parameters for next time
        m_InitialLowerThreshold = m_LowerThreshold;
        m_InitialUpperThreshold = m_UpperThreshold;

        if (m_FillFeedbackContour)
        {
          // 3. use contour to fill a region in our working slice
          Contour* feedbackContour( FeedbackContourTool::GetFeedbackContour() );
          if (feedbackContour)
          {
            Contour::Pointer projectedContour = FeedbackContourTool::ProjectContourTo2DSlice( m_WorkingSlice, feedbackContour, false, false ); // false: don't add any 0.5
                                                                                                                                    // false: don't constrain the contour to the image's inside
            if (projectedContour.IsNotNull())
            {
              FeedbackContourTool::FillContourInSlice( projectedContour, m_WorkingSlice, m_PaintingPixelValue );

              // 4. write working slice back into image volume
              int affectedDimension( -1 );
              int affectedSlice( -1 );
              const PlaneGeometry* planeGeometry( dynamic_cast<const PlaneGeometry*> (positionEvent->GetSender()->GetCurrentWorldGeometry2D() ) );
              FeedbackContourTool::DetermineAffectedImageSlice( dynamic_cast<Image*>( m_ToolManager->GetWorkingData(0)->GetData() ), planeGeometry, affectedDimension, affectedSlice );

              ToolLogger::Logger(3) << "OnMouseReleased: writing back to dimension " << affectedDimension << ", slice " << affectedSlice << " in working image" << std::endl;

              OverwriteSliceImageFilter::Pointer slicewriter = OverwriteSliceImageFilter::New();
              Image::Pointer workingImage = dynamic_cast<Image*>( m_ToolManager->GetWorkingData(0)->GetData() );
              slicewriter->SetInput( workingImage );
              slicewriter->SetCreateUndoInformation( true );
              slicewriter->SetSliceImage( m_WorkingSlice );
              slicewriter->SetSliceDimension( affectedDimension );
              slicewriter->SetSliceIndex( affectedSlice );
              slicewriter->SetTimeStep( positionEvent->GetSender()->GetTimeStep( workingImage ) );
              slicewriter->Update();
            }
          }
        }
        
        FeedbackContourTool::SetFeedbackContourVisible(false);
        mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );
      }
    }
  }

  m_ReferenceSlice = NULL; // don't leak
  m_WorkingSlice = NULL;  
  m_OriginalPicSlice = NULL;
    
  return true;
}

/**
Uses ipSegmentation algorithms to do the actual region growing. The result (binary image) is first smoothed by a 5x5 circle mask, then
its contour is extracted and converted to MITK coordinates.
*/
ipPicDescriptor* mitk::RegionGrowingTool::PerformRegionGrowingAndUpdateContour()
{
  // 1. m_OriginalPicSlice and m_SeedPointMemoryOffset are set to sensitive values, as well as m_LowerThreshold and m_UpperThreshold
  assert (m_OriginalPicSlice);
  if (m_OriginalPicSlice->n[0] != 256 || m_OriginalPicSlice->n[1] != 256) // ???
  assert( (m_SeedPointMemoryOffset < static_cast<int>( m_OriginalPicSlice->n[0] * m_OriginalPicSlice->n[1] )) && (m_SeedPointMemoryOffset >= 0) ); // inside the image

  // 2. ipSegmentation is used to perform region growing
  float ignored;
  int oneContourOffset( 0 );
  ipPicDescriptor* regionGrowerResult = ipMITKSegmentationGrowRegion4N( m_OriginalPicSlice,
                                                                        m_SeedPointMemoryOffset,       // seed point
                                                                        true,              // grayvalue interval relative to seed point gray value?
                                                                        m_LowerThreshold,
                                                                        m_UpperThreshold,
                                                                        0,                  // continue until done (maxIterations == 0)
                                                                        NULL,               // allocate new memory (only this time, on mouse move we'll reuse the old buffer)
                                                                        oneContourOffset,   // a pixel that is near the resulting contour
                                                                        ignored             // ignored by us
                                                                      );

  if (!regionGrowerResult || oneContourOffset == -1)
  {
    Contour::Pointer dummyContour = Contour::New();
    dummyContour->Initialize();
    FeedbackContourTool::SetFeedbackContour( *dummyContour );
    
    if (regionGrowerResult) ipMITKSegmentationFree(regionGrowerResult);
    return NULL;
  }

  // 3. We smooth the result a little to reduce contour complexity
  bool smoothResult( true ); // currently fixed, perhaps remove else block
  ipPicDescriptor* smoothedRegionGrowerResult;
  if (smoothResult)
  {
    // Smooth the result (otherwise very detailed contour)
    smoothedRegionGrowerResult = SmoothIPPicBinaryImage( regionGrowerResult, oneContourOffset );

    ipMITKSegmentationFree( regionGrowerResult );
  }
  else
  {
    smoothedRegionGrowerResult = regionGrowerResult;
  }

  // 4. convert the result of region growing into a mitk::Contour
  // At this point oneContourOffset could be useless, if smoothing destroyed a thin bridge. In these
  // cases, we have two or more unconnected segmentation regions, and we don't know, which one is touched by oneContourOffset.
  // In the bad case, the contour is not the one around our seedpoint, so the result looks very strange to the user.
  // -> we remove the point where the contour started so far. Then we look from the bottom of the image for the first segmentation pixel
  //    and start another contour extraction from there. This is done, until the seedpoint is inside the contour
  int numberOfContourPoints( 0 );
  int newBufferSize( 0 );
  float* contourPoints = ipMITKSegmentationGetContour8N( smoothedRegionGrowerResult, oneContourOffset, numberOfContourPoints, newBufferSize ); // memory allocated with malloc
  if (contourPoints)
  {
    while ( !ipMITKSegmentationIsInsideContour( contourPoints,                                               // contour
                                                numberOfContourPoints,                                       // points in contour
                                                m_SeedPointMemoryOffset % smoothedRegionGrowerResult->n[0],  // test point x
                                                m_SeedPointMemoryOffset / smoothedRegionGrowerResult->n[0]   // test point y
                                              ) )
    {
      // we decide that this cannot be part of the segmentation because the seedpoint is not contained in the contour (fill the 4-neighborhood with 0)
      ipMITKSegmentationReplaceRegion4N( smoothedRegionGrowerResult, oneContourOffset, 0 );

      // move the contour offset to the last row (x position of the seed point)
      int rowLength = smoothedRegionGrowerResult->n[0]; // number of pixels in a row
      oneContourOffset =    m_SeedPointMemoryOffset % smoothedRegionGrowerResult->n[0]  // x of seed point
                          + rowLength*(smoothedRegionGrowerResult->n[1]-1);              // y of last row

      while (    oneContourOffset >=0 
              && (*(static_cast<ipMITKSegmentationTYPE*>(smoothedRegionGrowerResult->data) + oneContourOffset) == 0) )
      {
        oneContourOffset -= rowLength; // if pixel at data+oneContourOffset is 0, then move up one row
      }
      
      if ( oneContourOffset < 0 )
      {
        break; // just use the last contour we found
      }
    
      free(contourPoints); // release contour memory
      contourPoints = ipMITKSegmentationGetContour8N( smoothedRegionGrowerResult, oneContourOffset, numberOfContourPoints, newBufferSize ); // memory allocated with malloc
    }

    // copy point from float* to mitk::Contour 
    Contour::Pointer contourInImageIndexCoordinates = Contour::New();
    contourInImageIndexCoordinates->Initialize();
    Point3D newPoint;
    for (int index = 0; index < numberOfContourPoints; ++index)
    {
      newPoint[0] = contourPoints[ 2 * index + 0 ];
      newPoint[1] = contourPoints[ 2 * index + 1];
      newPoint[2] = 0;

      contourInImageIndexCoordinates->AddVertex( newPoint );
    }

    free(contourPoints);

    Contour::Pointer contourInWorldCoordinates = FeedbackContourTool::BackProjectContourFrom2DSlice( m_ReferenceSlice, contourInImageIndexCoordinates, true );   // true: sub 0.5 for ipSegmentation correctio

    FeedbackContourTool::SetFeedbackContour( *contourInWorldCoordinates );
  }

  // 5. Result HAS TO BE freed by caller, contains the binary region growing result
  return smoothedRegionGrowerResult; 
}

/**
  Helper method for SmoothIPPicBinaryImage. Smoothes a given part of and image.

  \param sourceImage The original binary image.
  \param dest The smoothed image (will be written without bounds checking).
  \param contourOfs One offset of the contour. Is updated if a pixel is changed (which might change the contour).
  \param maskOffsets Memory offsets that describe the smoothing mask. 
  \param maskSize Entries of the mask.
  \param startOffset First pixel that should be smoothed using this mask.
  \param endOffset Last pixel that should be smoothed using this mask.
*/
void mitk::RegionGrowingTool::SmoothIPPicBinaryImageHelperForRows( ipPicDescriptor* sourceImage, ipPicDescriptor* dest, int &contourOfs, int* maskOffsets, int maskSize, int startOffset, int endOffset )
{
  // work on the very first row
  ipMITKSegmentationTYPE* current;
  ipMITKSegmentationTYPE* source = ((ipMITKSegmentationTYPE*)sourceImage->data) + startOffset; // + 1! don't read at start-1
  ipMITKSegmentationTYPE* end = ((ipMITKSegmentationTYPE*)dest->data) + endOffset;
  int ofs = startOffset;
  int minority = (maskSize - 1) / 2;

  for (current = ((ipMITKSegmentationTYPE*)dest->data) + startOffset; current<end; current++) 
  {
    ipInt1_t sum( 0 );
    for (int i = 0; i < maskSize; ++i) 
    {
      sum += *(source+maskOffsets[i]);
    }

    if (sum > minority) 
    {
      *current = 1;
      contourOfs = ofs;
    }
    else 
    {
      *current = 0;
    }

    ++source;
    ++ofs;
  }
}

/**
Smoothes a binary ipPic image with a 5x5 mask. The image borders (some first and last rows) are treated differently.
*/
ipPicDescriptor* mitk::RegionGrowingTool::SmoothIPPicBinaryImage( ipPicDescriptor* image, int &contourOfs, ipPicDescriptor* dest )
{
  if (!image) return NULL;

  // Original code from /trunk/mbi-qm/Qmitk/Qmitk2DSegTools/RegionGrowerTool.cpp (first version by T. Boettger?). Reformatted and documented and restructured.
  #define MSK_SIZE5x5 21
  #define MSK_SIZE3x3 5
  #define MSK_SIZE3x1 3 
  
  // mask is an array of coordinates that form a rastered circle like this
  //
  //            OOO  
  //           OOOOO 
  //           OOOOO 
  //           OOOOO 
  //            OOO  
  //
  //
  int mask5x5[MSK_SIZE5x5][2] 
  = {
  /******/ {-1,-2}, {0,-2}, {1,-2}, /*****/ 
  {-2,-1}, {-1,-1}, {0,-1}, {1,-1}, {2,-1},
  {-2, 0}, {-1, 0}, {0, 0}, {1, 0}, {2, 0},
  {-2, 1}, {-1, 1}, {0, 1}, {1, 1}, {2, 1},
  /******/ {-1, 2}, {0, 2}, {1, 2}  /*****/ 
  };

  int mask3x3[MSK_SIZE3x3][2] 
  = {
  /******/ {0,-1}, /*****/ 
  {-1, 0}, {0, 0}, {1, 0},
  /******/ {0, 1} /*****/ 
  };

  int mask3x1[MSK_SIZE3x1][2] 
  = {
  {-1, 0}, {0, 0}, {1, 0}
  };


  // The following lines iterate over all the pixels of a (sliced) image (except the first and last three rows).
  // For each pixel, all the coordinates around it (according to mask) are evaluated (this means 21 pixels).
  // If more than 10 of the evaluated pixels are non-zero, then the central pixel is set to 1, else to 0.
  // This is determining a majority. If there is no clear majority, then the central pixel itself "decides".
  int maskOffset5x5[MSK_SIZE5x5];
  int line = image->n[0];
  for (int i=0; i<MSK_SIZE5x5; i++) 
  {
    maskOffset5x5[i] = mask5x5[i][0] + line * mask5x5[i][1]; // calculate memory offsets from the x,y mask elements
  }

  int maskOffset3x3[MSK_SIZE3x3];
  for (int i=0; i<MSK_SIZE3x3; i++) 
  {
    maskOffset3x3[i] = mask3x3[i][0] + line * mask3x3[i][1]; // calculate memory offsets from the x,y mask elements
  }

  int maskOffset3x1[MSK_SIZE3x1];
  for (int i=0; i<MSK_SIZE3x1; i++) 
  {
    maskOffset3x1[i] = mask3x1[i][0] + line * mask3x1[i][1]; // calculate memory offsets from the x,y mask elements
  }


  if (!dest) 
  {
    // create pic if necessary
    dest = ipMITKSegmentationNew( image );
  }

  int spareOut3Rows = 3*image->n[0];
  int spareOut1Rows = 1*image->n[0];

  if ( image->n[1] > 0 ) SmoothIPPicBinaryImageHelperForRows( image, dest, contourOfs, maskOffset3x1, MSK_SIZE3x1, 1, dest->n[0] );
  if ( image->n[1] > 3 ) SmoothIPPicBinaryImageHelperForRows( image, dest, contourOfs, maskOffset3x3, MSK_SIZE3x3, spareOut1Rows, dest->n[0]*3 );
  if ( image->n[1] > 6 ) SmoothIPPicBinaryImageHelperForRows( image, dest, contourOfs, maskOffset5x5, MSK_SIZE5x5, spareOut3Rows, dest->n[0]*dest->n[1] - spareOut3Rows );
  if ( image->n[1] > 8 ) SmoothIPPicBinaryImageHelperForRows( image, dest, contourOfs, maskOffset3x3, MSK_SIZE3x3, dest->n[0]*dest->n[1] -spareOut3Rows, dest->n[0]*dest->n[1] - spareOut1Rows );
  if ( image->n[1] > 10) SmoothIPPicBinaryImageHelperForRows( image, dest, contourOfs, maskOffset3x1, MSK_SIZE3x1, dest->n[0]*dest->n[1] -spareOut1Rows, dest->n[0]*dest->n[1] - 1 );

  // correction for first pixel (sorry for the ugliness)
  if ( *((ipMITKSegmentationTYPE*)(dest->data)+1) == 1 ) 
  {
    *((ipMITKSegmentationTYPE*)(dest->data)+0) = 1;
  }

  if (dest->n[0] * dest->n[1] > 2)
  {
    // correction for last pixel
    if ( *((ipMITKSegmentationTYPE*)(dest->data)+dest->n[0]*dest->n[1]-2) == 1 ) 
    {
      *((ipMITKSegmentationTYPE*)(dest->data)+dest->n[0]*dest->n[1]-1) = 1;
    }
  }
  
  return dest;
}


