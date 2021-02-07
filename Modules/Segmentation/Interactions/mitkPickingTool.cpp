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

#include <itkConnectedThresholdImageFilter.h>

#include <mitkLabelSetImage.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, PickingTool, "PickingTool");
}

mitk::PickingTool::PickingTool() : m_WorkingData(nullptr)
{
  m_PointSetNode = mitk::DataNode::New();
  m_PointSetNode->GetPropertyList()->SetProperty("name", mitk::StringProperty::New("Picking_Seedpoint"));
  m_PointSetNode->GetPropertyList()->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_PointSet = mitk::PointSet::New();
  m_PointSetNode->SetData(m_PointSet);

  // Watch for point added or modified
  itk::SimpleMemberCommand<PickingTool>::Pointer pointAddedCommand = itk::SimpleMemberCommand<PickingTool>::New();
  pointAddedCommand->SetCallbackFunction(this, &mitk::PickingTool::OnPointAdded);
  m_PointSetAddObserverTag = m_PointSet->AddObserver(mitk::PointSetAddEvent(), pointAddedCommand);

  // create new node for picked region
  m_ResultNode = mitk::DataNode::New();
  // set some properties
  m_ResultNode->SetProperty("name", mitk::StringProperty::New("result"));
  m_ResultNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_ResultNode->SetProperty("color", mitk::ColorProperty::New(0, 1, 0));
  m_ResultNode->SetProperty("layer", mitk::IntProperty::New(1));
  m_ResultNode->SetProperty("opacity", mitk::FloatProperty::New(0.33f));
}

mitk::PickingTool::~PickingTool()
{
  m_PointSet->RemoveObserver(m_PointSetAddObserverTag);
}

bool mitk::PickingTool::CanHandle(const BaseData* referenceData, const BaseData* workingData) const
{
  if (!Superclass::CanHandle(referenceData,workingData))
    return false;

  auto* image = dynamic_cast<const Image*>(referenceData);

  if (image == nullptr)
    return false;

  if (image->GetTimeSteps() > 1) //release quickfix for T28248
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

  DataStorage *dataStorage = this->GetDataStorage();
  m_WorkingData = this->GetWorkingData();

  // add to datastorage and enable interaction
  if (!dataStorage->Exists(m_PointSetNode))
    dataStorage->Add(m_PointSetNode, m_WorkingData);

  m_SeedPointInteractor = mitk::SinglePointDataInteractor::New();
  m_SeedPointInteractor->LoadStateMachine("PointSet.xml");
  m_SeedPointInteractor->SetEventConfig("PointSetConfig.xml");
  m_SeedPointInteractor->SetDataNode(m_PointSetNode);

  // now add result to data tree
  dataStorage->Add(m_ResultNode, m_WorkingData);
}

void mitk::PickingTool::Deactivated()
{
  m_PointSet->Clear();
  // remove from data storage and disable interaction
  GetDataStorage()->Remove(m_PointSetNode);
  GetDataStorage()->Remove(m_ResultNode);

  Superclass::Deactivated();
}

mitk::DataNode *mitk::PickingTool::GetReferenceData()
{
  return this->m_ToolManager->GetReferenceData(0);
}

mitk::DataStorage *mitk::PickingTool::GetDataStorage()
{
  return this->m_ToolManager->GetDataStorage();
}

mitk::DataNode *mitk::PickingTool::GetWorkingData()
{
  return this->m_ToolManager->GetWorkingData(0);
}

mitk::DataNode::Pointer mitk::PickingTool::GetPointSetNode()
{
  return m_PointSetNode;
}

void mitk::PickingTool::OnPointAdded()
{
  if (m_WorkingData != this->GetWorkingData())
  {
    DataStorage *dataStorage = this->GetDataStorage();

    if (dataStorage->Exists(m_PointSetNode))
    {
      dataStorage->Remove(m_PointSetNode);
      dataStorage->Add(m_PointSetNode, this->GetWorkingData());
    }

    if (dataStorage->Exists(m_ResultNode))
    {
      dataStorage->Remove(m_ResultNode);
      dataStorage->Add(m_ResultNode, this->GetWorkingData());
    }

    m_WorkingData = this->GetWorkingData();
  }

  // Perform region growing/picking

  int timeStep =
    mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget0"))->GetTimeStep();

  mitk::PointSet::PointType seedPoint = m_PointSet->GetPointSet(timeStep)->GetPoints()->Begin().Value();

  // as we want to pick a region from our segmentation image use the working data from ToolManager
  mitk::Image::Pointer orgImage = dynamic_cast<mitk::Image *>(m_ToolManager->GetWorkingData(0)->GetData());

  if (orgImage.IsNotNull())
  {
    if (orgImage->GetDimension() == 4)
    { // there may be 4D segmentation data even though we currently don't support that
      mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
      timeSelector->SetInput(orgImage);
      timeSelector->SetTimeNr(timeStep);
      timeSelector->UpdateLargestPossibleRegion();
      mitk::Image *timedImage = timeSelector->GetOutput();

      AccessByItk_2(timedImage, StartRegionGrowing, timedImage->GetGeometry(), seedPoint);
    }
    else if (orgImage->GetDimension() == 3)
    {
      AccessByItk_2(orgImage, StartRegionGrowing, orgImage->GetGeometry(), seedPoint);
    }
    this->m_PointSet->Clear();
  }
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::PickingTool::StartRegionGrowing(itk::Image<TPixel, VImageDimension> *itkImage,
                                           mitk::BaseGeometry *imageGeometry,
                                           mitk::PointSet::PointType seedPoint)
{
  typedef itk::Image<TPixel, VImageDimension> InputImageType;
  typedef typename InputImageType::IndexType IndexType;
  typedef itk::ConnectedThresholdImageFilter<InputImageType, InputImageType> RegionGrowingFilterType;
  typename RegionGrowingFilterType::Pointer regionGrower = RegionGrowingFilterType::New();

  // convert world coordinates to image indices
  IndexType seedIndex;
  imageGeometry->WorldToIndex(seedPoint, seedIndex);

  // perform region growing in desired segmented region
  regionGrower->SetInput(itkImage);
  regionGrower->AddSeed(seedIndex);

  // TODO: conversion added to silence warning and
  // maintain existing behaviour, should be fixed
  // since it's not correct e.g. for signed char
  regionGrower->SetLower(static_cast<typename InputImageType::PixelType>(1));
  regionGrower->SetUpper(static_cast<typename InputImageType::PixelType>(255));

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

  // Store result and preview
  mitk::Image::Pointer resultImage = mitk::ImportItkImage(regionGrower->GetOutput(), imageGeometry)->Clone();
  mitk::LabelSetImage::Pointer resultLabelSetImage = mitk::LabelSetImage::New();
  resultLabelSetImage->InitializeByLabeledImage(resultImage);

  m_ResultNode->SetData(resultLabelSetImage);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::PickingTool::ConfirmSegmentation()
{
  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
  newNode->SetProperty("name", mitk::StringProperty::New(m_WorkingData->GetName() + "_picked"));

  float rgb[3] = {1.0f, 0.0f, 0.0f};
  m_WorkingData->GetColor(rgb);
  newNode->SetProperty("color", mitk::ColorProperty::New(rgb));

  float opacity = 1.0f;
  m_WorkingData->GetOpacity(opacity, nullptr);
  newNode->SetProperty("opacity", mitk::FloatProperty::New(opacity));

  newNode->SetData(m_ResultNode->GetData());

  GetDataStorage()->Add(newNode, this->GetReferenceData());
  m_WorkingData->SetVisibility(false);

  m_ResultNode->SetData(nullptr);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
