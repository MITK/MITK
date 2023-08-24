/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPickingTool.h"

#include "mitkProperties.h"
#include "mitkToolManager.h"

#include "mitkInteractionPositionEvent.h"
// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

#include "mitkITKImageImport.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"
#include "mitkImageTimeSelector.h"

#include <itkImage.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkOrImageFilter.h>
#include <mitkLabelSetImage.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, PickingTool, "PickingTool");
}

mitk::PickingTool::PickingTool() : SegWithPreviewTool(false, "PressMoveReleaseAndPointSetting")
{
  this->ResetsToEmptyPreviewOn();
}

mitk::PickingTool::~PickingTool()
{
}

const char **mitk::PickingTool::GetXPM() const
{
  return nullptr;
}

const char *mitk::PickingTool::GetName() const
{
  return "Picking";
}

us::ModuleResource mitk::PickingTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Picking.svg");
  return resource;
}

void mitk::PickingTool::Activated()
{
  Superclass::Activated();

  m_PointSet = mitk::PointSet::New();
  //ensure that the seed points are visible for all timepoints.
  dynamic_cast<ProportionalTimeGeometry*>(m_PointSet->GetTimeGeometry())->SetStepDuration(std::numeric_limits<TimePointType>::max());

  m_PointSetNode = mitk::DataNode::New();
  m_PointSetNode->SetData(m_PointSet);
  m_PointSetNode->SetName(std::string(this->GetName()) + "_PointSet");
  m_PointSetNode->SetBoolProperty("helper object", true);
  m_PointSetNode->SetColor(0.0, 1.0, 0.0);
  m_PointSetNode->SetVisibility(true);

  this->GetDataStorage()->Add(m_PointSetNode, this->GetToolManager()->GetWorkingData(0));
}

void mitk::PickingTool::Deactivated()
{
  this->ClearSeeds();

  // remove from data storage and disable interaction
  GetDataStorage()->Remove(m_PointSetNode);
  m_PointSetNode = nullptr;
  m_PointSet = nullptr;

  Superclass::Deactivated();
}

void mitk::PickingTool::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("ShiftSecondaryButtonPressed", OnAddPoint);
  CONNECT_FUNCTION("ShiftPrimaryButtonPressed", OnAddPoint);
  CONNECT_FUNCTION("DeletePoint", OnDelete);
}

void mitk::PickingTool::OnAddPoint(StateMachineAction*, InteractionEvent* interactionEvent)
{
  if (!this->IsUpdating() && m_PointSet.IsNotNull())
  {
    const auto positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);

    if (positionEvent != nullptr)
    {
      m_PointSet->InsertPoint(m_PointSet->GetSize(), positionEvent->GetPositionInWorld());
      this->UpdatePreview();
    }
  }
}

void mitk::PickingTool::OnDelete(StateMachineAction*, InteractionEvent* /*interactionEvent*/)
{
  if (!this->IsUpdating() && m_PointSet.IsNotNull())
  {
    // delete last seed point
    if (this->m_PointSet->GetSize() > 0)
    {
      m_PointSet->RemovePointAtEnd(0);

      this->UpdatePreview();
    }
  }
}

void mitk::PickingTool::ClearPicks()
{
  this->ClearSeeds();
  this->UpdatePreview();
}

bool mitk::PickingTool::HasPicks() const
{
  return this->m_PointSet.IsNotNull() && this->m_PointSet->GetSize()>0;
}

void mitk::PickingTool::ClearSeeds()
{
  if (this->m_PointSet.IsNotNull())
  {
    // renew pointset
    this->m_PointSet = mitk::PointSet::New();
    //ensure that the seed points are visible for all timepoints.
    dynamic_cast<ProportionalTimeGeometry*>(m_PointSet->GetTimeGeometry())->SetStepDuration(std::numeric_limits<TimePointType>::max());
    this->m_PointSetNode->SetData(this->m_PointSet);
  }
}

template <typename TPixel, unsigned int VImageDimension>
void DoITKRegionGrowing(const itk::Image<TPixel, VImageDimension>* oldSegImage,
  mitk::Image* segmentation,
  const mitk::PointSet* seedPoints,
  unsigned int timeStep, const mitk::BaseGeometry* inputGeometry, const mitk::Label::PixelType outputValue,
  const mitk::Label::PixelType backgroundValue,
  bool& emptyTimeStep)
{
  typedef itk::Image<TPixel, VImageDimension> InputImageType;
  typedef itk::Image<mitk::Label::PixelType, VImageDimension> OutputImageType;
  typedef typename InputImageType::IndexType IndexType;
  typedef itk::ConnectedThresholdImageFilter<InputImageType, OutputImageType> RegionGrowingFilterType;

  using IndexMapType = std::map < mitk::Label::PixelType, std::vector<IndexType> >;

  IndexMapType indexMap;

  // convert world coordinates to image indices
  for (auto pos = seedPoints->Begin(); pos != seedPoints->End(); ++pos)
  {
    IndexType seedIndex;
    inputGeometry->WorldToIndex(pos->Value(), seedIndex);
    const auto selectedLabel = oldSegImage->GetPixel(seedIndex);

    if (selectedLabel != backgroundValue)
    {
      indexMap[selectedLabel].push_back(seedIndex);
    }
  }

  typename OutputImageType::Pointer itkResultImage;

  try
  {
    bool first = true;
    typename RegionGrowingFilterType::Pointer regionGrower = RegionGrowingFilterType::New();
    regionGrower->SetInput(oldSegImage);
    regionGrower->SetReplaceValue(outputValue);

    for (const auto& [label, indeces] : indexMap)
    {
      // perform region growing in desired segmented region
      regionGrower->ClearSeeds();
      for (const auto& index : indeces)
      {
        regionGrower->AddSeed(index);
      }

      regionGrower->SetLower(label);
      regionGrower->SetUpper(label);

      regionGrower->Update();

      if (first)
      {
        itkResultImage = regionGrower->GetOutput();
      }
      else
      {
        typename itk::OrImageFilter<OutputImageType, OutputImageType>::Pointer orFilter =
          itk::OrImageFilter<OutputImageType, OutputImageType>::New();
        orFilter->SetInput1(regionGrower->GetOutput());
        orFilter->SetInput2(itkResultImage);

        orFilter->Update();
        itkResultImage = orFilter->GetOutput();
      }
      first = false;
      itkResultImage->DisconnectPipeline();
    }
  }
  catch (const itk::ExceptionObject&)
  {
    return; // can't work
  }
  catch (...)
  {
    return;
  }

  if (itkResultImage.IsNotNull())
  {
    segmentation->SetVolume((void*)(itkResultImage->GetPixelContainer()->GetBufferPointer()),timeStep);
  }
  emptyTimeStep = itkResultImage.IsNull();

}

void mitk::PickingTool::DoUpdatePreview(const Image* /*inputAtTimeStep*/, const Image* oldSegAtTimeStep, LabelSetImage* previewImage, TimeStepType timeStep)
{
  if (nullptr != oldSegAtTimeStep && nullptr != previewImage && m_PointSet.IsNotNull())
  {
    bool emptyTimeStep = true;
    if (this->HasPicks())
    {
      const auto activeValue = this->GetActiveLabelValueOfPreview();
      this->SetSelectedLabels({activeValue});

      AccessFixedDimensionByItk_n(oldSegAtTimeStep, DoITKRegionGrowing, 3, (previewImage, this->m_PointSet, timeStep, oldSegAtTimeStep->GetGeometry(), activeValue, mitk::LabelSetImage::UnlabeledValue, emptyTimeStep));
    }
    if (emptyTimeStep)
    {
      this->ResetPreviewContentAtTimeStep(timeStep);
    }
  }
}
