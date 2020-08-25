/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// MITK
#include "mitkOtsuTool3D.h"
#include "mitkImageAccessByItk.h"
#include "mitkLabelSetImage.h"
#include "mitkOtsuSegmentationFilter.h"
#include "mitkRenderingManager.h"
#include "mitkToolManager.h"
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkLevelWindowProperty.h>
#include <mitkLookupTableProperty.h>
#include <mitkRenderingModeProperty.h>
#include <mitkSliceNavigationController.h>

// ITK
#include <itkBinaryThresholdImageFilter.h>
#include <itkOrImageFilter.h>
#include <itkOtsuMultipleThresholdsImageFilter.h>

// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

#include <mitkImageStatisticsHolder.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, OtsuTool3D, "Otsu Segmentation");
}

mitk::OtsuTool3D::OtsuTool3D() : AutoSegmentationWithPreviewTool(true)
{
}

mitk::OtsuTool3D::~OtsuTool3D()
{
}

void mitk::OtsuTool3D::SetSelectedRegions(const SelectedRegionVectorType& regions)
{
  if (m_SelectedRegions != regions)
  {
    m_SelectedRegions = regions;
    //Note: we do not call this->Modified() on puprose. Reason: changing the
    //selected regions should not force to run otsu filter in DoUpdatePreview due to changed MTime.
  }
}

mitk::OtsuTool3D::SelectedRegionVectorType mitk::OtsuTool3D::GetSelectedRegions() const
{
  return this->m_SelectedRegions;
}

void mitk::OtsuTool3D::Activated()
{
  Superclass::Activated();

  m_SelectedRegions = {};
  m_NumberOfBins = 128;
  m_NumberOfRegions = 2;
  m_UseValley = false;

  m_OtsuResultNode = mitk::DataNode::New();
  m_OtsuResultNode->SetName("Otsu_Preview");
  // m_MultiLabelResultNode->SetBoolProperty("helper object", true);
  m_OtsuResultNode->SetVisibility(true);
  m_OtsuResultNode->SetOpacity(1.0);

  m_ToolManager->GetDataStorage()->Add(m_OtsuResultNode);
}

void mitk::OtsuTool3D::Deactivated()
{
  m_ToolManager->GetDataStorage()->Remove(m_OtsuResultNode);
  m_OtsuResultNode = nullptr;

  Superclass::Deactivated();
}

const char **mitk::OtsuTool3D::GetXPM() const
{
  return nullptr;
}

us::ModuleResource mitk::OtsuTool3D::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Otsu_48x48.png");
  return resource;
}

const char* mitk::OtsuTool3D::GetName() const
{
  return "Otsu";
}

void mitk::OtsuTool3D::UpdateCleanUp()
{
  if (m_OtsuResultNode.IsNotNull())
    m_OtsuResultNode->SetVisibility(m_SelectedRegions.empty());

  if (nullptr != this->GetPreviewSegmentationNode())
    this->GetPreviewSegmentationNode()->SetVisibility(!m_SelectedRegions.empty());

  if (m_SelectedRegions.empty())
  {
    this->ResetPreviewNode();
  }
}

void mitk::OtsuTool3D::DoUpdatePreview(const Image* inputAtTimeStep, Image* previewImage, TimeStepType timeStep)
{
  int numberOfThresholds = m_NumberOfRegions - 1;

  const auto timePoint = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
  mitk::LabelSetImage::Pointer otsuResultImage = dynamic_cast<LabelSetImage*>(this->m_OtsuResultNode->GetData());

  if (nullptr == m_OtsuResultNode->GetData()
      || this->GetMTime() > m_OtsuResultNode->GetData()->GetMTime()
      || this->m_LastOtsuTimeStep != timeStep //this covers the case where dynamic
                                              //segmentations have to compute a preview
                                              //for all time steps on confirmation
      || this->GetLastTimePointOfUpdate() != timePoint //this ensures that static seg
                                                       //previews work with dynamic images
                                                       //with avoiding unnecessary other otsu computations
     )
  {
    if (nullptr == inputAtTimeStep)
    {
      MITK_WARN << "Cannot run segementation. Currently selected input image is not set.";
      return;
    }

    this->m_LastOtsuTimeStep = timeStep;

    mitk::OtsuSegmentationFilter::Pointer otsuFilter = mitk::OtsuSegmentationFilter::New();
    otsuFilter->SetNumberOfThresholds(numberOfThresholds);
    otsuFilter->SetValleyEmphasis(m_UseValley);
    otsuFilter->SetNumberOfBins(m_NumberOfBins);
    otsuFilter->SetInput(inputAtTimeStep);
    otsuFilter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);

    try
    {
      otsuFilter->Update();
    }
    catch (...)
    {
      mitkThrow() << "itkOtsuFilter error (image dimension must be in {2, 3} and image must not be RGB)";
    }

    otsuResultImage = mitk::LabelSetImage::New();
    otsuResultImage->InitializeByLabeledImage(otsuFilter->GetOutput());
    this->m_OtsuResultNode->SetData(otsuResultImage);
    this->m_OtsuResultNode->SetProperty("binary", mitk::BoolProperty::New(false));
    mitk::RenderingModeProperty::Pointer renderingMode = mitk::RenderingModeProperty::New();
    renderingMode->SetValue(mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR);
    this->m_OtsuResultNode->SetProperty("Image Rendering.Mode", renderingMode);
    mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
    mitk::LookupTableProperty::Pointer prop = mitk::LookupTableProperty::New(lut);
    vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
    lookupTable->SetHueRange(1.0, 0.0);
    lookupTable->SetSaturationRange(1.0, 1.0);
    lookupTable->SetValueRange(1.0, 1.0);
    lookupTable->SetTableRange(-1.0, 1.0);
    lookupTable->Build();
    lut->SetVtkLookupTable(lookupTable);
    prop->SetLookupTable(lut);
    this->m_OtsuResultNode->SetProperty("LookupTable", prop);
    mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
    mitk::LevelWindow levelwindow;
    levelwindow.SetRangeMinMax(0, numberOfThresholds + 1);
    levWinProp->SetLevelWindow(levelwindow);
    this->m_OtsuResultNode->SetProperty("levelwindow", levWinProp);
  }

  if (!m_SelectedRegions.empty())
  {
    AccessByItk_n(otsuResultImage, CalculatePreview, (previewImage, timeStep));
  }
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::OtsuTool3D::CalculatePreview(itk::Image<TPixel, VImageDimension> *itkImage, mitk::Image* segmentation, unsigned int timeStep)
{
  typedef itk::Image<TPixel, VImageDimension> InputImageType;
  typedef itk::Image<mitk::Tool::DefaultSegmentationDataType, VImageDimension> OutputImageType;

  typedef itk::BinaryThresholdImageFilter<InputImageType, OutputImageType> FilterType;

  typename FilterType::Pointer filter = FilterType::New();

  // InputImageType::Pointer itkImage;
  typename OutputImageType::Pointer itkBinaryResultImage;

  filter->SetInput(itkImage);
  filter->SetLowerThreshold(m_SelectedRegions[0]);
  filter->SetUpperThreshold(m_SelectedRegions[0]);
  filter->SetInsideValue(1);
  filter->SetOutsideValue(0);
  filter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);
  filter->Update();
  itkBinaryResultImage = filter->GetOutput();
  itkBinaryResultImage->DisconnectPipeline();

  // if more than one region id is used compute the union of all given binary regions
  for (const auto regionID : m_SelectedRegions)
  {
    if (regionID != m_SelectedRegions[0])
    {
      filter->SetLowerThreshold(regionID);
      filter->SetUpperThreshold(regionID);
      filter->SetInsideValue(1);
      filter->SetOutsideValue(0);
      filter->Update();

      typename OutputImageType::Pointer tempImage = filter->GetOutput();

      typename itk::OrImageFilter<OutputImageType, OutputImageType>::Pointer orFilter =
        itk::OrImageFilter<OutputImageType, OutputImageType>::New();
      orFilter->SetInput1(tempImage);
      orFilter->SetInput2(itkBinaryResultImage);
      orFilter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);

      orFilter->UpdateLargestPossibleRegion();
      itkBinaryResultImage = orFilter->GetOutput();
    }
  }
  //----------------------------------------------------------------------------------------------------

  segmentation->SetVolume((void*)(itkBinaryResultImage->GetPixelContainer()->GetBufferPointer()), timeStep);
}

unsigned int mitk::OtsuTool3D::GetMaxNumberOfBins() const
{
  const auto min = this->GetReferenceData()->GetStatistics()->GetScalarValueMin();
  const auto max = this->GetReferenceData()->GetStatistics()->GetScalarValueMaxNoRecompute();
  return static_cast<unsigned int>(max - min) + 1;
}
