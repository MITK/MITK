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
#include "mitkImageTimeSelector.h"

#include <itkImage.h>
#include <itkConnectedThresholdImageFilter.h>

#include <mitkLabelSetImage.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, PickingTool, "PickingTool");
}

mitk::PickingTool::PickingTool() : AutoSegmentationWithPreviewTool(false, "PressMoveReleaseAndPointSetting")
{
  this->ResetsToEmptyPreviewOn();
}

mitk::PickingTool::~PickingTool()
{
}

bool mitk::PickingTool::CanHandle(const BaseData* referenceData, const BaseData* workingData) const
{
  if (!Superclass::CanHandle(referenceData,workingData))
    return false;

  auto* image = dynamic_cast<const Image*>(referenceData);

  if (image == nullptr)
    return false;

  return true;
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
  us::ModuleResource resource = module->GetResource("Pick_48x48.png");
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
  unsigned int timeStep, const mitk::BaseGeometry* inputGeometry)
{
  typedef itk::Image<TPixel, VImageDimension> InputImageType;
  typedef typename InputImageType::IndexType IndexType;
  typedef itk::ConnectedThresholdImageFilter<InputImageType, InputImageType> RegionGrowingFilterType;
  typename RegionGrowingFilterType::Pointer regionGrower = RegionGrowingFilterType::New();

  // convert world coordinates to image indices
  for (auto pos = seedPoints->Begin(); pos != seedPoints->End(); ++pos)
  {
    IndexType seedIndex;
    inputGeometry->WorldToIndex(pos->Value(), seedIndex);
    regionGrower->AddSeed(seedIndex);
  }

  // perform region growing in desired segmented region
  regionGrower->SetInput(oldSegImage);

  regionGrower->SetLower(static_cast<typename InputImageType::PixelType>(1));
  regionGrower->SetUpper(std::numeric_limits<typename InputImageType::PixelType>::max());

  try
  {
    regionGrower->Update();
  }
  catch (const itk::ExceptionObject &)
  {
    return; // can't work
  }
  catch (...)
  {
    return;
  }

  segmentation->SetVolume((void*)(regionGrower->GetOutput()->GetPixelContainer()->GetBufferPointer()), timeStep);
}

void mitk::PickingTool::DoUpdatePreview(const Image* /*inputAtTimeStep*/, const Image* oldSegAtTimeStep, Image* previewImage, TimeStepType timeStep)
{
  if (nullptr != oldSegAtTimeStep && nullptr != previewImage && m_PointSet.IsNotNull())
  {
    AccessFixedDimensionByItk_n(oldSegAtTimeStep, DoITKRegionGrowing, 3, (previewImage, this->m_PointSet, timeStep, oldSegAtTimeStep->GetGeometry()));
  }
}
