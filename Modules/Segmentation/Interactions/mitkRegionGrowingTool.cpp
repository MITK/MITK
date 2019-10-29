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

#include "mitkRegionGrowingTool.h"
#include "mitkApplicationCursor.h"
#include "mitkBaseRenderer.h"
#include "mitkImageDataItem.h"
#include "mitkImageToContourModelFilter.h"
#include "mitkOverwriteSliceImageFilter.h"
#include "mitkRegionGrowingTool.xpm"
#include "mitkRenderingManager.h"
#include "mitkToolManager.h"

#include "mitkExtractDirectedPlaneImageFilterNew.h"
#include "mitkLabelSetImage.h"
#include "mitkOverwriteDirectedPlaneImageFilter.h"

// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

// ITK
#include "mitkITKImageImport.h"
#include "mitkImageAccessByItk.h"
#include <itkConnectedComponentImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkNeighborhoodIterator.h>

#include <itkImageDuplicator.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, RegionGrowingTool, "Region growing tool");
}

#define ROUND(a) ((a) > 0 ? (int)((a) + 0.5) : -(int)(0.5 - (a)))

mitk::RegionGrowingTool::RegionGrowingTool()
  : FeedbackContourTool("PressMoveRelease"),
    m_SeedValue(0),
    m_ScreenYDifference(0),
    m_ScreenXDifference(0),
    m_MouseDistanceScaleFactor(0.5),
    m_PaintingPixelValue(0),
    m_FillFeedbackContour(true),
    m_ConnectedComponentValue(1)
{
}

mitk::RegionGrowingTool::~RegionGrowingTool()
{
}

void mitk::RegionGrowingTool::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("PrimaryButtonPressed", OnMousePressed);
  CONNECT_FUNCTION("Move", OnMouseMoved);
  CONNECT_FUNCTION("Release", OnMouseReleased);
}

const char **mitk::RegionGrowingTool::GetXPM() const
{
  return mitkRegionGrowingTool_xpm;
}

us::ModuleResource mitk::RegionGrowingTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("RegionGrowing_48x48.png");
  return resource;
}

us::ModuleResource mitk::RegionGrowingTool::GetCursorIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("RegionGrowing_Cursor_32x32.png");
  return resource;
}

const char *mitk::RegionGrowingTool::GetName() const
{
  return "Region Growing";
}

void mitk::RegionGrowingTool::Activated()
{
  Superclass::Activated();
}

void mitk::RegionGrowingTool::Deactivated()
{
  Superclass::Deactivated();
}

// Get the average pixel value of square/cube with radius=neighborhood around index
template <typename TPixel, unsigned int imageDimension>
void mitk::RegionGrowingTool::GetNeighborhoodAverage(itk::Image<TPixel, imageDimension> *itkImage,
                                                     itk::Index<imageDimension> index,
                                                     ScalarType *result,
                                                     unsigned int neighborhood)
{
  // maybe assert that image dimension is only 2 or 3?
  auto neighborhoodInt = (int)neighborhood;
  TPixel averageValue(0);
  unsigned int numberOfPixels = (2 * neighborhood + 1) * (2 * neighborhood + 1);
  if (imageDimension == 3)
  {
    numberOfPixels *= (2 * neighborhood + 1);
  }

  MITK_DEBUG << "Getting neighborhood of " << numberOfPixels << " pixels around " << index;

  itk::Index<imageDimension> currentIndex;

  for (int i = (0 - neighborhoodInt); i <= neighborhoodInt; ++i)
  {
    currentIndex[0] = index[0] + i;

    for (int j = (0 - neighborhoodInt); j <= neighborhoodInt; ++j)
    {
      currentIndex[1] = index[1] + j;

      if (imageDimension == 3)
      {
        for (int k = (0 - neighborhoodInt); k <= neighborhoodInt; ++k)
        {
          currentIndex[2] = index[2] + k;

          if (itkImage->GetLargestPossibleRegion().IsInside(currentIndex))
          {
            averageValue += itkImage->GetPixel(currentIndex);
          }
          else
          {
            numberOfPixels -= 1;
          }
        }
      }
      else
      {
        if (itkImage->GetLargestPossibleRegion().IsInside(currentIndex))
        {
          averageValue += itkImage->GetPixel(currentIndex);
        }
        else
        {
          numberOfPixels -= 1;
        }
      }
    }
  }

  *result = (ScalarType)averageValue;
  *result /= numberOfPixels;
}

// Check whether index lies inside a segmentation
template <typename TPixel, unsigned int imageDimension>
void mitk::RegionGrowingTool::IsInsideSegmentation(itk::Image<TPixel, imageDimension> *itkImage,
                                                   itk::Index<imageDimension> index,
                                                   bool *result)
{
  if (itkImage->GetPixel(index) > 0)
  {
    *result = true;
  }
  else
  {
    *result = false;
  }
}

// Do the region growing (i.e. call an ITK filter that does it)
template <typename TPixel, unsigned int imageDimension>
void mitk::RegionGrowingTool::StartRegionGrowing(itk::Image<TPixel, imageDimension> *inputImage,
                                                 itk::Index<imageDimension> seedIndex,
                                                 std::array<ScalarType, 2> thresholds,
                                                 mitk::Image::Pointer &outputImage)
{
  MITK_DEBUG << "Starting region growing at index " << seedIndex << " with lower threshold " << thresholds[0]
             << " and upper threshold " << thresholds[1];

  typedef itk::Image<TPixel, imageDimension> InputImageType;
  typedef itk::Image<DefaultSegmentationDataType, imageDimension> OutputImageType;

  typedef itk::ConnectedThresholdImageFilter<InputImageType, OutputImageType> RegionGrowingFilterType;
  typename RegionGrowingFilterType::Pointer regionGrower = RegionGrowingFilterType::New();

  // perform region growing in desired segmented region
  regionGrower->SetInput(inputImage);
  regionGrower->AddSeed(seedIndex);

  regionGrower->SetLower(thresholds[0]);
  regionGrower->SetUpper(thresholds[1]);

  try
  {
    regionGrower->Update();
  }
  catch (...)
  {
    return; // Should we do something?
  }

  typename OutputImageType::Pointer resultImage = regionGrower->GetOutput();

  // Smooth result: Every pixel is replaced by the majority of the neighborhood
  typedef itk::NeighborhoodIterator<OutputImageType> NeighborhoodIteratorType;
  typedef itk::ImageRegionIterator<OutputImageType> ImageIteratorType;

  typename NeighborhoodIteratorType::RadiusType radius;
  radius.Fill(2); // for now, maybe make this something the user can adjust in the preferences?

  typedef itk::ImageDuplicator< OutputImageType > DuplicatorType;
  typename DuplicatorType::Pointer duplicator = DuplicatorType::New();
  duplicator->SetInputImage(resultImage);
  duplicator->Update();

  typename OutputImageType::Pointer resultDup = duplicator->GetOutput();

  NeighborhoodIteratorType neighborhoodIterator(radius, resultDup, resultDup->GetRequestedRegion());
  ImageIteratorType imageIterator(resultImage, resultImage->GetRequestedRegion());

  for (neighborhoodIterator.GoToBegin(), imageIterator.GoToBegin(); !neighborhoodIterator.IsAtEnd();
       ++neighborhoodIterator, ++imageIterator)
  {
    DefaultSegmentationDataType voteYes(0);
    DefaultSegmentationDataType voteNo(0);

    for (unsigned int i = 0; i < neighborhoodIterator.Size(); ++i)
    {
      if (neighborhoodIterator.GetPixel(i) > 0)
      {
        voteYes += 1;
      }
      else
      {
        voteNo += 1;
      }
    }

    if (voteYes > voteNo)
    {
      imageIterator.Set(1);
    }
    else
    {
      imageIterator.Set(0);
    }
  }

  if (resultImage.IsNull())
  {
    MITK_DEBUG << "Region growing result is empty.";
  }

  // Can potentially have multiple regions, use connected component image filter to label disjunct regions
  typedef itk::ConnectedComponentImageFilter<OutputImageType, OutputImageType> ConnectedComponentImageFilterType;
  typename ConnectedComponentImageFilterType::Pointer connectedComponentFilter =
    ConnectedComponentImageFilterType::New();
  connectedComponentFilter->SetInput(resultImage);
  connectedComponentFilter->Update();
  typename OutputImageType::Pointer resultImageCC = connectedComponentFilter->GetOutput();
  m_ConnectedComponentValue = resultImageCC->GetPixel(seedIndex);

  outputImage = mitk::GrabItkImageMemory(resultImageCC);
}

void mitk::RegionGrowingTool::OnMousePressed(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
  if (!positionEvent)
    return;

  MITK_DEBUG << "OnMousePressed";

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();
  m_LastScreenPosition = positionEvent->GetPointerPositionOnScreen();

  // ReferenceSlice is from the underlying image, WorkingSlice from the active segmentation (can be empty)
  m_ReferenceSlice = FeedbackContourTool::GetAffectedReferenceSlice(positionEvent);
  m_WorkingSlice = FeedbackContourTool::GetAffectedWorkingSlice(positionEvent);

  if (m_WorkingSlice.IsNotNull()) // can't do anything without a working slice (i.e. a possibly empty segmentation)
  {
    MITK_DEBUG << "OnMousePressed: got working slice";

    // 2. Determine if the user clicked inside or outside of the segmentation/working slice (i.e. the whole volume)
    mitk::BaseGeometry::Pointer workingSliceGeometry;
    workingSliceGeometry = m_WorkingSlice->GetGeometry();
    workingSliceGeometry->WorldToIndex(positionEvent->GetPositionInWorld(), m_SeedPoint);
    itk::Index<2> indexInWorkingSlice2D;
    indexInWorkingSlice2D[0] = m_SeedPoint[0];
    indexInWorkingSlice2D[1] = m_SeedPoint[1];

    if (workingSliceGeometry->IsIndexInside(m_SeedPoint))
    {
      MITK_DEBUG << "OnMousePressed: point " << positionEvent->GetPositionInWorld() << " (index coordinates "
                 << m_SeedPoint << ") is inside working slice";

      // 3. determine the pixel value under the last click to determine what to do
      bool inside(true);
      AccessFixedDimensionByItk_2(m_WorkingSlice, IsInsideSegmentation, 2, indexInWorkingSlice2D, &inside);
      m_PaintingPixelValue = inside ? 0 : 1;

      if (inside)
      {
        MITK_DEBUG << "Clicked inside segmentation";
        // For now, we're doing nothing when the user clicks inside the segmentation. Behaviour can be implemented via
        // OnMousePressedInside()
        // When you do, be sure to remove the m_PaintingPixelValue check in OnMouseMoved() and OnMouseReleased()
        return;
      }
      else
      {
        MITK_DEBUG << "Clicked outside of segmentation";
        OnMousePressedOutside(nullptr, interactionEvent);
      }
    }
  }
}

// Use this to implement a behaviour for when the user clicks inside a segmentation (for example remove something)
// Old IpPic code is kept as comment for reference
void mitk::RegionGrowingTool::OnMousePressedInside()
{
  //    mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent
  //    );
  //    //const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent()); // checked in
  //    OnMousePressed
  //    // 3.1.1. Create a skeletonization of the segmentation and try to find a nice cut
  //    // apply the skeletonization-and-cut algorithm
  //    // generate contour to remove
  //    // set m_ReferenceSlice = nullptr so nothing will happen during mouse move
  //    // remember to fill the contour with 0 in mouserelease
  //    mitkIpPicDescriptor* segmentationHistory = ipMITKSegmentationCreateGrowerHistory( workingPicSlice,
  //    m_LastWorkingSeed, nullptr ); // free again
  //    if (segmentationHistory)
  //    {
  //        tCutResult cutContour = ipMITKSegmentationGetCutPoints( workingPicSlice, segmentationHistory,
  //        initialWorkingOffset ); // tCutResult is a ipSegmentation type
  //        mitkIpPicFree( segmentationHistory );
  //        if (cutContour.cutIt)
  //        {
  //            int timestep = positionEvent->GetSender()->GetTimeStep();
  //            // 3.1.2 copy point from float* to mitk::Contour
  //            ContourModel::Pointer contourInImageIndexCoordinates = ContourModel::New();
  //            contourInImageIndexCoordinates->Expand(timestep + 1);
  //            contourInImageIndexCoordinates->SetClosed(true, timestep);
  //            Point3D newPoint;
  //            for (int index = 0; index < cutContour.deleteSize; ++index)
  //            {
  //                newPoint[0] = cutContour.deleteCurve[ 2 * index + 0 ] - 0.5;//correction is needed because the
  //                output of the algorithm is center based
  //                newPoint[1] = cutContour.deleteCurve[ 2 * index + 1 ] - 0.5;//and we want our contour displayed
  //                corner based.
  //                newPoint[2] = 0.0;

  //                contourInImageIndexCoordinates->AddVertex( newPoint, timestep );
  //            }

  //            free(cutContour.traceline);
  //            free(cutContour.deleteCurve); // perhaps visualize this for fun?
  //            free(cutContour.onGradient);

  //            ContourModel::Pointer contourInWorldCoordinates = FeedbackContourTool::BackProjectContourFrom2DSlice(
  //            m_WorkingSlice->GetGeometry(), contourInImageIndexCoordinates, true ); // true: sub 0.5 for
  //            ipSegmentation correction

  //            FeedbackContourTool::SetFeedbackContour( contourInWorldCoordinates );
  //            FeedbackContourTool::SetFeedbackContourVisible(true);
  //            mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );
  //            m_FillFeedbackContour = true;
  //        }
  //        else
  //        {
  //            m_FillFeedbackContour = false;
  //        }

  //    }
  //    else
  //    {
  //        m_FillFeedbackContour = false;
  //    }

  //    m_ReferenceSlice = nullptr;

  //    return true;
}

void mitk::RegionGrowingTool::OnMousePressedOutside(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);

  if (positionEvent)
  {
    // Get geometry and indices
    mitk::BaseGeometry::Pointer workingSliceGeometry;
    workingSliceGeometry = m_WorkingSlice->GetGeometry();
    itk::Index<2> indexInWorkingSlice2D;
    indexInWorkingSlice2D[0] = m_SeedPoint[0];
    indexInWorkingSlice2D[1] = m_SeedPoint[1];

    mitk::BaseGeometry::Pointer referenceSliceGeometry;
    referenceSliceGeometry =
      m_ReferenceSlice->GetGeometry();
    itk::Index<3> indexInReferenceSlice;
    itk::Index<2> indexInReferenceSlice2D;
    referenceSliceGeometry->WorldToIndex(positionEvent->GetPositionInWorld(), indexInReferenceSlice);
    indexInReferenceSlice2D[0] = indexInReferenceSlice[0];
    indexInReferenceSlice2D[1] = indexInReferenceSlice[1];

    // Get seed neighborhood
    ScalarType averageValue(0);
    AccessFixedDimensionByItk_3(m_ReferenceSlice, GetNeighborhoodAverage, 2, indexInReferenceSlice2D, &averageValue, 1);
    m_SeedValue = averageValue;
    MITK_DEBUG << "Seed value is " << m_SeedValue;

    // Get level window settings
    LevelWindow lw(0, 500); // default window 0 to 500, can we do something smarter here?
    m_ToolManager->GetReferenceData(0)->GetLevelWindow(
      lw); // will fill lw if levelwindow property is present, otherwise won't touch it.
    ScalarType currentVisibleWindow = lw.GetWindow();
    MITK_DEBUG << "Level window width is " << currentVisibleWindow;
    m_InitialThresholds[0] = m_SeedValue - currentVisibleWindow / 20.0; // 20 is arbitrary (though works reasonably
                                                                        // well), is there a better alternative (maybe
                                                                        // option in preferences)?
    m_InitialThresholds[1] = m_SeedValue + currentVisibleWindow / 20.0;
    m_Thresholds[0] = m_InitialThresholds[0];
    m_Thresholds[1] = m_InitialThresholds[1];

    // Perform region growing
    mitk::Image::Pointer resultImage = mitk::Image::New();
    AccessFixedDimensionByItk_3(
      m_ReferenceSlice, StartRegionGrowing, 2, indexInWorkingSlice2D, m_Thresholds, resultImage);
    resultImage->SetGeometry(workingSliceGeometry);

    // Extract contour
    if (resultImage.IsNotNull() && m_ConnectedComponentValue >= 1)
    {
      float isoOffset = 0.33;

      mitk::ImageToContourModelFilter::Pointer contourExtractor = mitk::ImageToContourModelFilter::New();
      contourExtractor->SetInput(resultImage);
      contourExtractor->SetContourValue(m_ConnectedComponentValue - isoOffset);
      contourExtractor->Update();
      ContourModel::Pointer resultContour = ContourModel::New();
      resultContour = contourExtractor->GetOutput();

      // Show contour
      if (resultContour.IsNotNull())
      {
        ContourModel::Pointer resultContourWorld = FeedbackContourTool::BackProjectContourFrom2DSlice(
          workingSliceGeometry, FeedbackContourTool::ProjectContourTo2DSlice(m_WorkingSlice, resultContour));

        // this is not a beautiful solution, just one that works, check T22412 for details
        auto t = positionEvent->GetSender()->GetTimeStep();

        FeedbackContourTool::SetFeedbackContour(0 != t
          ? ContourModelUtils::MoveZerothContourTimeStep(resultContourWorld, t)
          : resultContourWorld);

        FeedbackContourTool::SetFeedbackContourVisible(true);
        mitk::RenderingManager::GetInstance()->RequestUpdate(m_LastEventSender->GetRenderWindow());
      }
    }
  }
}

void mitk::RegionGrowingTool::OnMouseMoved(StateMachineAction *, InteractionEvent *interactionEvent)
{
  // Until OnMousePressedInside() implements a behaviour, we're just returning here whenever m_PaintingPixelValue is 0,
  // i.e. when the user clicked inside the segmentation
  if (m_PaintingPixelValue == 0)
  {
    return;
  }

  auto *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);

  if (m_ReferenceSlice.IsNotNull() && positionEvent)
  {
    // Get geometry and indices
    mitk::BaseGeometry::Pointer workingSliceGeometry;
    workingSliceGeometry = m_WorkingSlice->GetGeometry();
    itk::Index<2> indexInWorkingSlice2D;
    indexInWorkingSlice2D[0] = m_SeedPoint[0];
    indexInWorkingSlice2D[1] = m_SeedPoint[1];

    m_ScreenYDifference += positionEvent->GetPointerPositionOnScreen()[1] - m_LastScreenPosition[1];
    m_ScreenXDifference += positionEvent->GetPointerPositionOnScreen()[0] - m_LastScreenPosition[0];
    m_LastScreenPosition = positionEvent->GetPointerPositionOnScreen();

    // Moving the mouse up and down adjusts the width of the threshold window, moving it left and right shifts the
    // threshold window
    m_Thresholds[0] = std::min<ScalarType>(
      m_SeedValue, m_InitialThresholds[0] - (m_ScreenYDifference - m_ScreenXDifference) * m_MouseDistanceScaleFactor);
    m_Thresholds[1] = std::max<ScalarType>(
      m_SeedValue, m_InitialThresholds[1] + (m_ScreenYDifference + m_ScreenXDifference) * m_MouseDistanceScaleFactor);
    MITK_DEBUG << "Screen difference X: " << m_ScreenXDifference;

    // Perform region growing again and show the result
    mitk::Image::Pointer resultImage = mitk::Image::New();
    AccessFixedDimensionByItk_3(
      m_ReferenceSlice, StartRegionGrowing, 2, indexInWorkingSlice2D, m_Thresholds, resultImage);
    resultImage->SetGeometry(workingSliceGeometry);

    // Update the contour
    if (resultImage.IsNotNull() && m_ConnectedComponentValue >= 1)
    {
      float isoOffset = 0.33;

      mitk::ImageToContourModelFilter::Pointer contourExtractor = mitk::ImageToContourModelFilter::New();
      contourExtractor->SetInput(resultImage);
      contourExtractor->SetContourValue(m_ConnectedComponentValue - isoOffset);
      contourExtractor->Update();
      ContourModel::Pointer resultContour = ContourModel::New();
      resultContour = contourExtractor->GetOutput();

      // Show contour
      if (resultContour.IsNotNull())
      {
        ContourModel::Pointer resultContourWorld = FeedbackContourTool::BackProjectContourFrom2DSlice(
          workingSliceGeometry, FeedbackContourTool::ProjectContourTo2DSlice(m_WorkingSlice, resultContour));

        // this is not a beautiful solution, just one that works, check T22412 for details
        int timestep = positionEvent->GetSender()->GetTimeStep();
        if (0 != timestep)
        {
          int size = resultContourWorld->GetNumberOfVertices(0);
          auto resultContourTimeWorld = mitk::ContourModel::New();
          resultContourTimeWorld->Expand(timestep + 1);
          for (int loop = 0; loop < size; ++loop)
          {
            resultContourTimeWorld->AddVertex(resultContourWorld->GetVertexAt(loop, 0), timestep);
          }
          FeedbackContourTool::SetFeedbackContour(resultContourTimeWorld);
        }
        else
        {
          FeedbackContourTool::SetFeedbackContour(resultContourWorld);
        }

        FeedbackContourTool::SetFeedbackContourVisible(true);
        mitk::RenderingManager::GetInstance()->ForceImmediateUpdate(positionEvent->GetSender()->GetRenderWindow());
      }
    }
  }
}

void mitk::RegionGrowingTool::OnMouseReleased(StateMachineAction *, InteractionEvent *interactionEvent)
{
  // Until OnMousePressedInside() implements a behaviour, we're just returning here whenever m_PaintingPixelValue is 0,
  // i.e. when the user clicked inside the segmentation
  if (m_PaintingPixelValue == 0)
  {
    return;
  }

  auto *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);

  if (m_WorkingSlice.IsNotNull() && m_FillFeedbackContour && positionEvent)
  {
    // Project contour into working slice
    ContourModel *feedbackContour(FeedbackContourTool::GetFeedbackContour());

    ContourModel::Pointer projectedContour;

    // this is not a beautiful solution, just one that works, check T22412 for details
    int timestep = positionEvent->GetSender()->GetTimeStep();
    if (0 != timestep)
    {
      int size = feedbackContour->GetNumberOfVertices(timestep);
      auto feedbackContourTime = mitk::ContourModel::New();
      feedbackContourTime->Expand(timestep + 1);
      for (int loop = 0; loop < size; ++loop)
      {
        feedbackContourTime->AddVertex(feedbackContour->GetVertexAt(loop, timestep), 0);
      }

      projectedContour =
        FeedbackContourTool::ProjectContourTo2DSlice(m_WorkingSlice, feedbackContourTime, false, false);
    }
    else
    {
      projectedContour =
        FeedbackContourTool::ProjectContourTo2DSlice(m_WorkingSlice, feedbackContour, false, false);
    }

    // If there is a projected contour, fill it
    if (projectedContour.IsNotNull())
    {
      // Get working data to pass to following method so we don't overwrite locked labels in a LabelSetImage
      mitk::DataNode *workingNode(m_ToolManager->GetWorkingData(0));
      mitk::LabelSetImage *labelImage = workingNode != nullptr
        ? dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData())
        : nullptr;

      MITK_DEBUG << "Filling Segmentation";

      if (labelImage != nullptr)
      {
        // m_PaintingPixelValue only decides whether to paint or not
        // For LabelSetImages we want to paint with the active label value
        auto activeLabel = labelImage->GetActiveLabel(labelImage->GetActiveLayer())->GetValue();
        mitk::ContourModelUtils::FillContourInSlice(projectedContour,
                                                    0,
                                                    m_WorkingSlice,
                                                    labelImage,
                                                    m_PaintingPixelValue * activeLabel);
      }
      else
      {
        mitk::ContourModelUtils::FillContourInSlice(projectedContour,
                                                    0,
                                                    m_WorkingSlice,
                                                    m_WorkingSlice,
                                                    m_PaintingPixelValue);
      }
      this->WriteBackSegmentationResult(positionEvent, m_WorkingSlice);
      FeedbackContourTool::SetFeedbackContourVisible(false);
    }

    m_ScreenYDifference = 0;
    m_ScreenXDifference = 0;
  }
}
