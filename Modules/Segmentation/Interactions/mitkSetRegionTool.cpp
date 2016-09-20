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

#include "mitkBaseRenderer.h"
#include "mitkImageDataItem.h"
#include "mitkLegacyAdaptors.h"
#include "mitkLabelSetImage.h"

#include <mitkITKImageImport.h>
#include <mitkImageToContourModelFilter.h>
#include <mitkImagePixelReadAccessor.h>

#include <itkBinaryFillholeImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>

mitk::SetRegionTool::SetRegionTool(int paintingPixelValue)
:FeedbackContourTool("PressMoveRelease"),
 m_PaintingPixelValue(paintingPixelValue)
{
}

mitk::SetRegionTool::~SetRegionTool()
{
}

void mitk::SetRegionTool::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION( "PrimaryButtonPressed", OnMousePressed);
  CONNECT_FUNCTION( "Release", OnMouseReleased);
  CONNECT_FUNCTION( "Move",OnMouseMoved);
}

void mitk::SetRegionTool::Activated()
{
  Superclass::Activated();
}

void mitk::SetRegionTool::Deactivated()
{
  Superclass::Deactivated();
}

void mitk::SetRegionTool::OnMousePressed ( StateMachineAction*, InteractionEvent* interactionEvent )
{
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  if (!positionEvent) return;

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  // 1. Get the working image
  Image::Pointer workingSlice   = FeedbackContourTool::GetAffectedWorkingSlice( positionEvent );
  if ( workingSlice.IsNull() ) return; // can't do anything without the segmentation

  // if click was outside the image, don't continue
  const BaseGeometry* sliceGeometry = workingSlice->GetGeometry();
  itk::Index<3> projectedPointIn2D;
  sliceGeometry->WorldToIndex( positionEvent->GetPositionInWorld(), projectedPointIn2D );
  if ( !sliceGeometry->IsIndexInside( projectedPointIn2D ) )
  {
    MITK_ERROR << "point apparently not inside segmentation slice" << std::endl;
    return; // can't use that as a seed point
  }

  typedef itk::Image<DefaultSegmentationDataType, 2> InputImageType;
  typedef InputImageType::IndexType IndexType;
  typedef itk::ConnectedThresholdImageFilter<InputImageType, InputImageType> RegionGrowingFilterType;
  RegionGrowingFilterType::Pointer regionGrower = RegionGrowingFilterType::New();

  // convert world coordinates to image indices
  IndexType seedIndex;
  sliceGeometry->WorldToIndex( positionEvent->GetPositionInWorld(), seedIndex);

  //perform region growing in desired segmented region
  InputImageType::Pointer itkImage = InputImageType::New();
  CastToItkImage(workingSlice, itkImage);
  regionGrower->SetInput( itkImage );
  regionGrower->AddSeed( seedIndex );

  InputImageType::PixelType bound = itkImage->GetPixel(seedIndex);

  regionGrower->SetLower( bound );
  regionGrower->SetUpper( bound );
  regionGrower->SetReplaceValue(1);

  itk::BinaryFillholeImageFilter<InputImageType>::Pointer fillHolesFilter =
      itk::BinaryFillholeImageFilter<InputImageType>::New();

  fillHolesFilter->SetInput(regionGrower->GetOutput());
  fillHolesFilter->SetForegroundValue(1);

//<<<<<<< HEAD
  //Store result and preview
  mitk::Image::Pointer resultImage = mitk::GrabItkImageMemory(fillHolesFilter->GetOutput());
  resultImage->SetGeometry(workingSlice->GetGeometry());
//=======
  // Get the current working color
  DataNode* workingNode( m_ToolManager->GetWorkingData(0) );
  if (!workingNode) return;

  Image* image = dynamic_cast<Image*>(workingNode->GetData());
  LabelSetImage* labelImage = dynamic_cast<LabelSetImage*>(image);
  int activeColor = 1;
  if (labelImage != 0)
  {
    activeColor = labelImage->GetActiveLabel()->GetValue();
  }


//  // 2. Determine the contour that surronds the selected "piece of the image"
//
//  // find a contour seed point
//  unsigned int oneContourOffset = static_cast<unsigned int>( m_SeedPointMemoryOffset ); // safe because of earlier check if m_SeedPointMemoryOffset < 0
//
//  /**
//    * The logic of finding a starting point for the contour is the following:
//    *
//    *  - If the initial seed point is 0, we are either inside a hole or outside of every segmentation.
//    *    We move to the right until we hit a 1, which must be part of a contour.
//    *
//    *  - If the initial seed point is 1, then ...
//    *    we now do the same (running to the right) until we hit a 1
//    *
//    *  In both cases the found contour point is used to extract a contour and
//    *  then a test is applied to find out if the initial seed point is contained
//    *  in the contour. If this is the case, filling should be applied, otherwise
//    *  nothing is done.
//    */
//  unsigned int size = originalPicSlice->n[0] * originalPicSlice->n[1];
///*
//  unsigned int rowSize = originalPicSlice->n[0];
//*/
//  ipMITKSegmentationTYPE* data = static_cast<ipMITKSegmentationTYPE*>(originalPicSlice->data);
//
//  if ( data[oneContourOffset] == 0 ) // initial seed 0
//  {
//    for ( ; oneContourOffset < size; ++oneContourOffset )
//    {
//      if ( data[oneContourOffset] > 0 ) break;
//    }
//  }
//  else if ( data[oneContourOffset] == activeColor ) // initial seed 1
//  {
//    unsigned int lastValidPixel = size-1; // initialization, will be changed lateron
//    bool inSeg = true;    // inside segmentation?
//    for ( ; oneContourOffset < size; ++oneContourOffset )
//    {
//      if ( ( data[oneContourOffset] != activeColor ) && inSeg ) // pixel 0 and inside-flag set: this happens at the first pixel outside a filled region
//      {
//        inSeg = false;
//        lastValidPixel = oneContourOffset - 1; // store the last pixel position inside a filled region
//        break;
//      }
//      else // pixel 1, inside-flag doesn't matter: this happens while we are inside a filled region
//      {
//        inSeg = true; // first iteration lands here
//      }
//
//    }
//    oneContourOffset = lastValidPixel;
//  }
//>>>>>>> origin/bug-18647-multilabelsegmentationplugin-integration-2015-11-25

  mitk::ImageToContourModelFilter::Pointer contourextractor = mitk::ImageToContourModelFilter::New();
  contourextractor->SetInput(resultImage);
  contourextractor->Update();

  mitk::ContourModel::Pointer awesomeContour = contourextractor->GetOutput();
  FeedbackContourTool::SetFeedbackContour( awesomeContour );
  FeedbackContourTool::SetFeedbackContourVisible(true);
  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
}

void mitk::SetRegionTool::OnMouseReleased( StateMachineAction*, InteractionEvent* interactionEvent )
{
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  if (!positionEvent) return;

  assert( positionEvent->GetSender()->GetRenderWindow() );
  // 1. Hide the feedback contour, find out which slice the user clicked, find out which slice of the toolmanager's working image corresponds to that
  FeedbackContourTool::SetFeedbackContourVisible(false);
  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());

  int timeStep = positionEvent->GetSender()->GetTimeStep();

  DataNode* workingNode( m_ToolManager->GetWorkingData(0) );
  if (!workingNode) return;

  Image* image = dynamic_cast<Image*>(workingNode->GetData());
  const PlaneGeometry* planeGeometry( (positionEvent->GetSender()->GetCurrentWorldPlaneGeometry() ) );
  if ( !image || !planeGeometry ) return;

  Image::Pointer slice = FeedbackContourTool::GetAffectedImageSliceAs2DImage( positionEvent, image );

  if ( slice.IsNull() )
  {
      MITK_ERROR << "Unable to extract slice." << std::endl;
      return;
  }

  ContourModel* feedbackContour( FeedbackContourTool::GetFeedbackContour() );
  ContourModel::Pointer projectedContour = FeedbackContourTool::ProjectContourTo2DSlice( slice, feedbackContour, false, false ); // false: don't add 0.5 (done by FillContourInSlice)
  // false: don't constrain the contour to the image's inside
  if (projectedContour.IsNull()) return;

  LabelSetImage* labelImage = dynamic_cast<LabelSetImage*>(image);
  int activeColor = 1;
  if (labelImage != 0)
  {
    activeColor = labelImage->GetActiveLabel()->GetValue();
  }

  mitk::ContourModelUtils::FillContourInSlice(projectedContour, timeStep, slice, image, m_PaintingPixelValue*activeColor);

  //FeedbackContourTool::FillContourInSlice( projectedContour, timeStep, slice, m_PaintingPixelValue );

  this->WriteBackSegmentationResult(positionEvent, slice);
}

void mitk::SetRegionTool::OnMouseMoved(mitk::StateMachineAction *, mitk::InteractionEvent *)
{
}
