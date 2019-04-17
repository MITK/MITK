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
#include "mitkLabelSetImage.h"

#include <mitkITKImageImport.h>
#include <mitkImagePixelReadAccessor.h>
#include <mitkImageToContourModelFilter.h>

#include <itkBinaryFillholeImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>

mitk::SetRegionTool::SetRegionTool(int paintingPixelValue)
  : FeedbackContourTool("PressMoveRelease"), m_PaintingPixelValue(paintingPixelValue)
{
}

mitk::SetRegionTool::~SetRegionTool()
{
}

void mitk::SetRegionTool::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("PrimaryButtonPressed", OnMousePressed);
  CONNECT_FUNCTION("Release", OnMouseReleased);
  CONNECT_FUNCTION("Move", OnMouseMoved);
}

void mitk::SetRegionTool::Activated()
{
  Superclass::Activated();
}

void mitk::SetRegionTool::Deactivated()
{
  Superclass::Deactivated();
}

void mitk::SetRegionTool::OnMousePressed(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
  if (!positionEvent)
    return;

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  // 1. Get the working image
  Image::Pointer workingSlice = FeedbackContourTool::GetAffectedWorkingSlice(positionEvent);
  if (workingSlice.IsNull())
    return; // can't do anything without the segmentation

  // if click was outside the image, don't continue
  const BaseGeometry *sliceGeometry = workingSlice->GetGeometry();
  itk::Index<3> projectedPointIn2D;
  sliceGeometry->WorldToIndex(positionEvent->GetPositionInWorld(), projectedPointIn2D);
  if (!sliceGeometry->IsIndexInside(projectedPointIn2D))
  {
    MITK_WARN << "Point outside of segmentation slice." << std::endl;
    return; // can't use that as a seed point
  }

  typedef itk::Image<DefaultSegmentationDataType, 2> InputImageType;
  typedef InputImageType::IndexType IndexType;
  typedef itk::ConnectedThresholdImageFilter<InputImageType, InputImageType> RegionGrowingFilterType;
  RegionGrowingFilterType::Pointer regionGrower = RegionGrowingFilterType::New();

  // convert world coordinates to image indices
  IndexType seedIndex;
  sliceGeometry->WorldToIndex(positionEvent->GetPositionInWorld(), seedIndex);

  // perform region growing in desired segmented region
  InputImageType::Pointer itkImage = InputImageType::New();
  CastToItkImage(workingSlice, itkImage);
  regionGrower->SetInput(itkImage);
  regionGrower->AddSeed(seedIndex);

  InputImageType::PixelType bound = itkImage->GetPixel(seedIndex);

  regionGrower->SetLower(bound);
  regionGrower->SetUpper(bound);
  regionGrower->SetReplaceValue(1);

  itk::BinaryFillholeImageFilter<InputImageType>::Pointer fillHolesFilter =
    itk::BinaryFillholeImageFilter<InputImageType>::New();

  fillHolesFilter->SetInput(regionGrower->GetOutput());
  fillHolesFilter->SetForegroundValue(1);

  // Store result and preview
  mitk::Image::Pointer resultImage = mitk::GrabItkImageMemory(fillHolesFilter->GetOutput());
  resultImage->SetGeometry(workingSlice->GetGeometry());
  // Get the current working color
  DataNode *workingNode(m_ToolManager->GetWorkingData(0));
  if (!workingNode)
    return;

  mitk::ImageToContourModelFilter::Pointer contourextractor = mitk::ImageToContourModelFilter::New();
  contourextractor->SetInput(resultImage);
  contourextractor->Update();

  mitk::ContourModel::Pointer awesomeContour = contourextractor->GetOutput();
  auto t = positionEvent->GetSender()->GetTimeStep();

  FeedbackContourTool::SetFeedbackContour(0 != t
    ? ContourModelUtils::MoveZerothContourTimeStep(awesomeContour, t)
    : awesomeContour);

  FeedbackContourTool::SetFeedbackContourVisible(true);
  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
}

void mitk::SetRegionTool::OnMouseReleased(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
  if (!positionEvent)
    return;

  assert(positionEvent->GetSender()->GetRenderWindow());
  // 1. Hide the feedback contour, find out which slice the user clicked, find out which slice of the toolmanager's
  // working image corresponds to that
  FeedbackContourTool::SetFeedbackContourVisible(false);
  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());

  int timeStep = positionEvent->GetSender()->GetTimeStep();

  DataNode *workingNode(m_ToolManager->GetWorkingData(0));
  if (!workingNode)
    return;

  auto *image = dynamic_cast<Image *>(workingNode->GetData());
  const PlaneGeometry *planeGeometry((positionEvent->GetSender()->GetCurrentWorldPlaneGeometry()));
  if (!image || !planeGeometry)
    return;

  Image::Pointer slice = FeedbackContourTool::GetAffectedImageSliceAs2DImage(positionEvent, image);

  if (slice.IsNull())
  {
    MITK_ERROR << "Unable to extract slice." << std::endl;
    return;
  }

  ContourModel *feedbackContour(FeedbackContourTool::GetFeedbackContour());
  ContourModel::Pointer projectedContour = FeedbackContourTool::ProjectContourTo2DSlice(
    slice, feedbackContour, false, false); // false: don't add 0.5 (done by FillContourInSlice)
  // false: don't constrain the contour to the image's inside
  if (projectedContour.IsNull())
    return;

  auto *labelImage = dynamic_cast<LabelSetImage *>(image);
  int activeColor = 1;
  if (labelImage != nullptr)
  {
    activeColor = labelImage->GetActiveLabel()->GetValue();
  }

  mitk::ContourModelUtils::FillContourInSlice(
    projectedContour, timeStep, slice, image, m_PaintingPixelValue * activeColor);

  this->WriteBackSegmentationResult(positionEvent, slice);
}

void mitk::SetRegionTool::OnMouseMoved(mitk::StateMachineAction *, mitk::InteractionEvent *)
{
}
