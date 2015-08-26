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

#include "mitkBinaryThresholdULTool.h"

#include "mitkToolManager.h"

#include "mitkLevelWindowProperty.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"

#include "mitkDataStorage.h"
#include "mitkRenderingManager.h"
#include <mitkSliceNavigationController.h>

#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageStatisticsHolder.h"
#include "mitkImageTimeSelector.h"
#include <itkImageRegionIterator.h>
#include <itkBinaryThresholdImageFilter.h>
#include "mitkMaskAndCutRoiImageFilter.h"
#include "mitkPadImageFilter.h"

// us
#include "usModule.h"
#include "usModuleResource.h"
#include "usGetModuleContext.h"
#include "usModuleContext.h"

namespace mitk {
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, BinaryThresholdULTool, "ThresholdingUL tool");
}

mitk::BinaryThresholdULTool::BinaryThresholdULTool()
  :m_SensibleMinimumThresholdValue(-100),
  m_SensibleMaximumThresholdValue(+100),
  m_CurrentLowerThresholdValue(1),
  m_CurrentUpperThresholdValue(1)
{
  m_ThresholdFeedbackNode = DataNode::New();
  m_ThresholdFeedbackNode->SetProperty( "color", ColorProperty::New(0.0, 1.0, 0.0) );
  m_ThresholdFeedbackNode->SetProperty( "name", StringProperty::New("Thresholding feedback") );
  m_ThresholdFeedbackNode->SetProperty( "opacity", FloatProperty::New(0.3) );
  m_ThresholdFeedbackNode->SetProperty("binary", BoolProperty::New(true));
  m_ThresholdFeedbackNode->SetProperty( "helper object", BoolProperty::New(true) );
}

mitk::BinaryThresholdULTool::~BinaryThresholdULTool()
{
}

const char** mitk::BinaryThresholdULTool::GetXPM() const
{
  return NULL;
}

us::ModuleResource mitk::BinaryThresholdULTool::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("TwoThresholds_48x48.png");
  return resource;
}

const char* mitk::BinaryThresholdULTool::GetName() const
{
  return "UL Threshold";
}

void mitk::BinaryThresholdULTool::Activated()
{
  m_ToolManager->RoiDataChanged += mitk::MessageDelegate<mitk::BinaryThresholdULTool>(this, &mitk::BinaryThresholdULTool::OnRoiDataChanged);

  m_OriginalImageNode = m_ToolManager->GetReferenceData(0);
  m_NodeForThresholding = m_OriginalImageNode;

  if ( m_NodeForThresholding.IsNotNull() )
  {
    SetupPreviewNode();
  }
  else
  {
    m_ToolManager->ActivateTool(-1);
  }
}

void mitk::BinaryThresholdULTool::Deactivated()
{
  m_ToolManager->RoiDataChanged -= mitk::MessageDelegate<mitk::BinaryThresholdULTool>(this, &mitk::BinaryThresholdULTool::OnRoiDataChanged);
  m_NodeForThresholding = NULL;
  m_OriginalImageNode = NULL;
  try
  {
    if (DataStorage* storage = m_ToolManager->GetDataStorage())
    {
      storage->Remove( m_ThresholdFeedbackNode );
      RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
  catch(...)
  {
    // don't care
  }
  m_ThresholdFeedbackNode->SetData(NULL);
}

void mitk::BinaryThresholdULTool::SetThresholdValues(double lower, double upper)
{
  if (m_ThresholdFeedbackNode.IsNotNull())
  {
    m_CurrentLowerThresholdValue = lower;
    m_CurrentUpperThresholdValue = upper;
    UpdatePreview();
  }
}

void mitk::BinaryThresholdULTool::AcceptCurrentThresholdValue()
{

  CreateNewSegmentationFromThreshold(m_NodeForThresholding);

  RenderingManager::GetInstance()->RequestUpdateAll();
  m_ToolManager->ActivateTool(-1);
}

void mitk::BinaryThresholdULTool::CancelThresholding()
{
  m_ToolManager->ActivateTool(-1);
}

void mitk::BinaryThresholdULTool::SetupPreviewNode()
{
  if (m_NodeForThresholding.IsNotNull())
  {
    Image::Pointer image = dynamic_cast<Image*>( m_NodeForThresholding->GetData() );
    Image::Pointer originalImage = dynamic_cast<Image*> (m_OriginalImageNode->GetData());

    if (image.IsNotNull())
    {
      mitk::Image* workingimage = dynamic_cast<mitk::Image*>(m_ToolManager->GetWorkingData(0)->GetData());

      if(workingimage)
        m_ThresholdFeedbackNode->SetData( workingimage->Clone() );
      else
        m_ThresholdFeedbackNode->SetData( mitk::Image::New() );

      int layer(50);
      m_NodeForThresholding->GetIntProperty("layer", layer);
      m_ThresholdFeedbackNode->SetIntProperty("layer", layer+1);

      if (DataStorage* ds = m_ToolManager->GetDataStorage())
      {
        if (!ds->Exists(m_ThresholdFeedbackNode))
          ds->Add( m_ThresholdFeedbackNode, m_OriginalImageNode );
      }

      if (image.GetPointer() == originalImage.GetPointer())
      {
        Image::StatisticsHolderPointer statistics = originalImage->GetStatistics();
        m_SensibleMinimumThresholdValue = static_cast<double>( statistics->GetScalarValueMin() );
        m_SensibleMaximumThresholdValue = static_cast<double>( statistics->GetScalarValueMax() );
      }

      double range = m_SensibleMaximumThresholdValue - m_SensibleMinimumThresholdValue;
      m_CurrentLowerThresholdValue = m_SensibleMinimumThresholdValue + range/3.0;
      m_CurrentUpperThresholdValue = m_SensibleMinimumThresholdValue + 2*range/3.0;

      bool isFloatImage = false;
      if ((originalImage->GetPixelType().GetPixelType() == itk::ImageIOBase::SCALAR)
          &&(originalImage->GetPixelType().GetComponentType() == itk::ImageIOBase::FLOAT || originalImage->GetPixelType().GetComponentType() == itk::ImageIOBase::DOUBLE))
      {
        isFloatImage = true;
      }

      IntervalBordersChanged.Send(m_SensibleMinimumThresholdValue, m_SensibleMaximumThresholdValue, isFloatImage);
      ThresholdingValuesChanged.Send(m_CurrentLowerThresholdValue, m_CurrentUpperThresholdValue);
    }
  }
}

template <typename TPixel, unsigned int VImageDimension>
static void ITKSetVolume( itk::Image<TPixel, VImageDimension>* originalImage, mitk::Image* segmentation, unsigned int timeStep )
{
  segmentation->SetVolume( (void*) originalImage->GetPixelContainer()->GetBufferPointer(), timeStep);
}

void mitk::BinaryThresholdULTool::CreateNewSegmentationFromThreshold(DataNode* node)
{
  if (node)
  {
    Image::Pointer feedBackImage = dynamic_cast<Image*>( m_ThresholdFeedbackNode->GetData() );
    if (feedBackImage.IsNotNull())
    {
      // create a new image of the same dimensions and smallest possible pixel type
      DataNode::Pointer emptySegmentation = GetTargetSegmentationNode();

      if (emptySegmentation)
      {
        // actually perform a thresholding and ask for an organ type
        for (unsigned int timeStep = 0; timeStep < feedBackImage->GetTimeSteps(); ++timeStep)
        {
          try
          {
            ImageTimeSelector::Pointer timeSelector = ImageTimeSelector::New();
            timeSelector->SetInput( feedBackImage );
            timeSelector->SetTimeNr( timeStep );
            timeSelector->UpdateLargestPossibleRegion();
            Image::Pointer feedBackImage3D = timeSelector->GetOutput();

            if (feedBackImage3D->GetDimension() == 2)
            {
              AccessFixedDimensionByItk_2( feedBackImage3D, ITKSetVolume, 2, dynamic_cast<Image*>(emptySegmentation->GetData()), timeStep );
            }
            else
            {
              AccessFixedDimensionByItk_2( feedBackImage3D, ITKSetVolume, 3, dynamic_cast<Image*>(emptySegmentation->GetData()), timeStep );
            }

          }
          catch(...)
          {
            Tool::ErrorMessage("Error accessing single time steps of the original image. Cannot create segmentation.");
          }
        }

        //since we are maybe working on a smaller image, pad it to the size of the original image
        if (m_OriginalImageNode.GetPointer() != m_NodeForThresholding.GetPointer())
        {
          mitk::PadImageFilter::Pointer padFilter = mitk::PadImageFilter::New();

          padFilter->SetInput(0, dynamic_cast<mitk::Image*> (emptySegmentation->GetData()));
          padFilter->SetInput(1, dynamic_cast<mitk::Image*> (m_OriginalImageNode->GetData()));
          padFilter->SetBinaryFilter(true);
          padFilter->SetUpperThreshold(1);
          padFilter->SetLowerThreshold(1);
          padFilter->Update();

          emptySegmentation->SetData(padFilter->GetOutput());
        }

        m_ToolManager->SetWorkingData( emptySegmentation );
        m_ToolManager->GetWorkingData(0)->Modified();
      }
    }
  }
}



void mitk::BinaryThresholdULTool::OnRoiDataChanged()
{
  mitk::DataNode::Pointer node = m_ToolManager->GetRoiData(0);

  if (node.IsNotNull())
  {
    mitk::MaskAndCutRoiImageFilter::Pointer roiFilter = mitk::MaskAndCutRoiImageFilter::New();
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*> (m_NodeForThresholding->GetData());

    if (image.IsNull())
      return;

    roiFilter->SetInput(image);
    roiFilter->SetRegionOfInterest(node->GetData());
    roiFilter->Update();

    mitk::DataNode::Pointer tmpNode = mitk::DataNode::New();
    tmpNode->SetData(roiFilter->GetOutput());

    m_SensibleMinimumThresholdValue = static_cast<double>( roiFilter->GetMinValue());
    m_SensibleMaximumThresholdValue = static_cast<double>( roiFilter->GetMaxValue());

    m_NodeForThresholding = tmpNode;
  }
  else
    m_NodeForThresholding = m_OriginalImageNode;

  this->SetupPreviewNode();
  this->UpdatePreview();
}

template <typename TPixel, unsigned int VImageDimension>
static void ITKThresholding( itk::Image<TPixel, VImageDimension>* originalImage, mitk::Image* segmentation, double lower, double upper, unsigned int timeStep )
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<mitk::Tool::DefaultSegmentationDataType, VImageDimension> SegmentationType;
  typedef itk::BinaryThresholdImageFilter<ImageType, SegmentationType> ThresholdFilterType;

  typename ThresholdFilterType::Pointer filter = ThresholdFilterType::New();
  filter->SetInput(originalImage);
  filter->SetLowerThreshold(lower);
  filter->SetUpperThreshold(upper);
  filter->SetInsideValue(1);
  filter->SetOutsideValue(0);
  filter->Update();

  segmentation->SetVolume( (void*) (filter->GetOutput()->GetPixelContainer()->GetBufferPointer()), timeStep);
}

void mitk::BinaryThresholdULTool::UpdatePreview()
{
  mitk::Image::Pointer thresholdImage = dynamic_cast<mitk::Image*> (m_NodeForThresholding->GetData());
  mitk::Image::Pointer previewImage = dynamic_cast<mitk::Image*> (m_ThresholdFeedbackNode->GetData());
  if(thresholdImage && previewImage)
  {
    for (unsigned int timeStep = 0; timeStep < thresholdImage->GetTimeSteps(); ++timeStep)
    {
      ImageTimeSelector::Pointer timeSelector = ImageTimeSelector::New();
      timeSelector->SetInput( thresholdImage );
      timeSelector->SetTimeNr( timeStep );
      timeSelector->UpdateLargestPossibleRegion();
      Image::Pointer feedBackImage3D = timeSelector->GetOutput();

      AccessByItk_n(feedBackImage3D, ITKThresholding, (previewImage, m_CurrentLowerThresholdValue, m_CurrentUpperThresholdValue, timeStep));
    }
    RenderingManager::GetInstance()->RequestUpdateAll();
  }
}
