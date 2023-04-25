/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkFillRegionBaseTool.h"
#include "mitkToolManager.h"

#include "mitkBaseRenderer.h"
#include "mitkDataStorage.h"

#include "mitkITKImageImport.h"
#include "mitkImageAccessByItk.h"

#include "mitkRenderingManager.h"

#include <itkConnectedThresholdImageFilter.h>

mitk::FillRegionBaseTool::FillRegionBaseTool() : SegTool2D("MouseReleaseOnly")
{
}

mitk::FillRegionBaseTool::~FillRegionBaseTool()
{
}

void mitk::FillRegionBaseTool::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("Release", OnClick);
}


template <typename TPixel, unsigned int VImageDimension>
void DoITKRegionGrowing(const itk::Image<TPixel, VImageDimension>* oldSegImage,
  mitk::Image::Pointer& filledRegionImage, itk::Index<VImageDimension> seedIndex, mitk::Label::PixelType& seedLabel )
{
  typedef itk::Image<TPixel, VImageDimension> InputImageType;
  typedef itk::Image<mitk::Label::PixelType, VImageDimension> OutputImageType;
  typedef itk::ConnectedThresholdImageFilter<InputImageType, OutputImageType> RegionGrowingFilterType;

  seedLabel = oldSegImage->GetPixel(seedIndex);

  typename OutputImageType::Pointer itkResultImage;
  filledRegionImage = nullptr;

  try
  {
    typename RegionGrowingFilterType::Pointer regionGrower = RegionGrowingFilterType::New();
    regionGrower->SetInput(oldSegImage);
    regionGrower->SetReplaceValue(1);
    regionGrower->AddSeed(seedIndex);

    regionGrower->SetLower(seedLabel);
    regionGrower->SetUpper(seedLabel);

    regionGrower->Update();
    
    itkResultImage = regionGrower->GetOutput();
  }
  catch (const itk::ExceptionObject&)
  {
    return; // can't work
  }
  catch (...)
  {
    return;
  }
  mitk::CastToMitkImage(itkResultImage, filledRegionImage);
}

void mitk::FillRegionBaseTool::OnClick(StateMachineAction*, InteractionEvent* interactionEvent)
{
  auto positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
  if (nullptr == positionEvent)
    return;

  auto labelSetImage = dynamic_cast<const LabelSetImage*>(this->GetWorkingData());
  if (nullptr == labelSetImage)
  {
    return;
  }

  if (!IsPositionEventInsideImageRegion(positionEvent, labelSetImage))
  {
    return;
  }

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  auto workingSlice = this->GetAffectedWorkingSlice(positionEvent);

  auto click = positionEvent->GetPositionInWorld();

  m_SeedLabelValue = 0;
  auto fillImage = this->GenerateFillImage(workingSlice, click, m_SeedLabelValue);

  if (fillImage.IsNull())
  {
    return; //nothing to fill;
  }

  if (labelSetImage->IsLabelLocked(m_SeedLabelValue) && m_SeedLabelValue!=labelSetImage->GetActiveLabel(labelSetImage->GetActiveLayer())->GetValue())
  {
    ErrorMessage.Send("Label of selected region is locked. Tool operation has no effect.");
    return;
  }

  this->PrepareFilling(workingSlice, click);

  //as fill region tools should always allow to manipulate active label
  //(that is what the user expects/knows when using tools so far:
  //the active label can always be changed even if locked)
  //we realize that by cloning the relevant label set and changing the lock state
  //this fillLabelSet is used for the transfer.
  auto fillLabelSet = labelSetImage->GetActiveLabelSet()->Clone();
  auto activeLabelClone = fillLabelSet->GetLabel(labelSetImage->GetActiveLabel(labelSetImage->GetActiveLayer())->GetValue());
  if (nullptr != activeLabelClone)
  {
    activeLabelClone->SetLocked(false);
  }

  TransferLabelContentAtTimeStep(fillImage, workingSlice, fillLabelSet, 0, LabelSetImage::UnlabeledValue, LabelSetImage::UnlabeledValue, false, { {1, m_FillLabelValue} }, m_MergeStyle);

  this->WriteBackSegmentationResult(positionEvent, workingSlice);

  mitk::RenderingManager::GetInstance()->RequestUpdate(positionEvent->GetSender()->GetRenderWindow());
}

mitk::Image::Pointer mitk::FillRegionBaseTool::GenerateFillImage(const Image* workingSlice, Point3D seedPoint, mitk::Label::PixelType& seedLabelValue) const
{
  itk::Index<2> seedIndex;
  workingSlice->GetGeometry()->WorldToIndex(seedPoint, seedIndex);

  Image::Pointer fillImage;

  AccessFixedDimensionByItk_n(workingSlice, DoITKRegionGrowing, 2, (fillImage, seedIndex, seedLabelValue));

  return fillImage;
}
