/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// MITK
#include "mitkAutoMLSegmentationWithPreviewTool.h"
#include "mitkImageAccessByItk.h"
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

mitk::AutoMLSegmentationWithPreviewTool::AutoMLSegmentationWithPreviewTool() : AutoSegmentationWithPreviewTool(true)
{
}

void mitk::AutoMLSegmentationWithPreviewTool::SetSelectedLabels(const SelectedLabelVectorType& regions)
{
  if (m_SelectedLabels != regions)
  {
    m_SelectedLabels = regions;
    //Note: we do not call this->Modified() on puprose. Reason: changing the
    //selected regions should not force to run otsu filter in DoUpdatePreview due to changed MTime.
  }
}

const mitk::LabelSetImage* mitk::AutoMLSegmentationWithPreviewTool::GetMLPreview() const
{
  if (m_MLPreviewNode.IsNotNull())
  {
    const auto mlPreviewImage = dynamic_cast<const LabelSetImage*>(this->m_MLPreviewNode->GetData());
    return mlPreviewImage;
  }

  return nullptr;
}

mitk::AutoMLSegmentationWithPreviewTool::SelectedLabelVectorType mitk::AutoMLSegmentationWithPreviewTool::GetSelectedLabels() const
{
  return this->m_SelectedLabels;
}

void mitk::AutoMLSegmentationWithPreviewTool::Activated()
{
  Superclass::Activated();

  m_SelectedLabels = {};

  m_MLPreviewNode = mitk::DataNode::New();
  m_MLPreviewNode->SetProperty("name", StringProperty::New(std::string(this->GetName()) + "ML preview"));
  m_MLPreviewNode->SetProperty("helper object", BoolProperty::New(true));
  m_MLPreviewNode->SetVisibility(true);
  m_MLPreviewNode->SetOpacity(1.0);

  m_ToolManager->GetDataStorage()->Add(m_MLPreviewNode);
}

void mitk::AutoMLSegmentationWithPreviewTool::Deactivated()
{
  m_ToolManager->GetDataStorage()->Remove(m_MLPreviewNode);
  m_MLPreviewNode = nullptr;

  Superclass::Deactivated();
}

void mitk::AutoMLSegmentationWithPreviewTool::UpdateCleanUp()
{
  if (m_MLPreviewNode.IsNotNull())
    m_MLPreviewNode->SetVisibility(m_SelectedLabels.empty());

  if (nullptr != this->GetPreviewSegmentationNode())
    this->GetPreviewSegmentationNode()->SetVisibility(!m_SelectedLabels.empty());

  if (m_SelectedLabels.empty())
  {
    this->ResetPreviewNode();
  }
}

void mitk::AutoMLSegmentationWithPreviewTool::DoUpdatePreview(const Image* inputAtTimeStep, Image* previewImage, TimeStepType timeStep)
{
  const auto timePoint = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();

  if (nullptr == m_MLPreviewNode->GetData()
      || this->GetMTime() > m_MLPreviewNode->GetData()->GetMTime()
      || this->m_LastMLTimeStep != timeStep //this covers the case where dynamic
                                            //segmentations have to compute a preview
                                            //for all time steps on confirmation
      || this->GetLastTimePointOfUpdate() != timePoint //this ensures that static seg
                                                       //previews work with dynamic images
                                                       //with avoiding unnecessary other computations
     )
  {
    if (nullptr == inputAtTimeStep)
    {
      MITK_WARN << "Cannot run segementation. Currently selected input image is not set.";
      return;
    }

    this->m_LastMLTimeStep = timeStep;

    auto newMLPreview = ComputeMLPreview(inputAtTimeStep, timeStep);

    if (newMLPreview.IsNotNull())
    {
      this->m_MLPreviewNode->SetData(newMLPreview);
      this->m_MLPreviewNode->SetProperty("binary", mitk::BoolProperty::New(false));
      mitk::RenderingModeProperty::Pointer renderingMode = mitk::RenderingModeProperty::New();
      renderingMode->SetValue(mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR);
      this->m_MLPreviewNode->SetProperty("Image Rendering.Mode", renderingMode);
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
      this->m_MLPreviewNode->SetProperty("LookupTable", prop);
      mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
      mitk::LevelWindow levelwindow;
      levelwindow.SetRangeMinMax(0, newMLPreview->GetScalarValueMax());
      levWinProp->SetLevelWindow(levelwindow);
      this->m_MLPreviewNode->SetProperty("levelwindow", levWinProp);
    }
  }

  if (!m_SelectedLabels.empty())
  {
    const auto mlPreviewImage = this->GetMLPreview();
    if (nullptr != mlPreviewImage)
    {
      AccessByItk_n(mlPreviewImage, CalculateMergedSimplePreview, (previewImage, timeStep));
    }
  }
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::AutoMLSegmentationWithPreviewTool::CalculateMergedSimplePreview(const itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image* segmentation, unsigned int timeStep)
{
  typedef itk::Image<TPixel, VImageDimension> InputImageType;
  typedef itk::Image<mitk::Tool::DefaultSegmentationDataType, VImageDimension> OutputImageType;

  typedef itk::BinaryThresholdImageFilter<InputImageType, OutputImageType> FilterType;

  typename FilterType::Pointer filter = FilterType::New();

  // InputImageType::Pointer itkImage;
  typename OutputImageType::Pointer itkBinaryResultImage;

  filter->SetInput(itkImage);
  filter->SetLowerThreshold(m_SelectedLabels[0]);
  filter->SetUpperThreshold(m_SelectedLabels[0]);
  filter->SetInsideValue(1);
  filter->SetOutsideValue(0);
  filter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);
  filter->Update();
  itkBinaryResultImage = filter->GetOutput();
  itkBinaryResultImage->DisconnectPipeline();

  // if more than one region id is used compute the union of all given binary regions
  for (const auto labelID : m_SelectedLabels)
  {
    if (labelID != m_SelectedLabels[0])
    {
      filter->SetLowerThreshold(labelID);
      filter->SetUpperThreshold(labelID);
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
