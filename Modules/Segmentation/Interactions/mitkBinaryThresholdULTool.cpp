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
#include "mitkBinaryThresholdULTool.xpm"

#include "mitkToolManager.h"

#include "mitkLevelWindowProperty.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"

#include "mitkDataStorage.h"
#include "mitkRenderingManager.h"

#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageTimeSelector.h"
#include <itkImageRegionIterator.h>
#include <itkBinaryThresholdImageFilter.h>
#include "mitkMaskAndCutRoiImageFilter.h"
#include "mitkPadImageFilter.h"

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, BinaryThresholdULTool, "ThresholdingUL tool");
}

mitk::BinaryThresholdULTool::BinaryThresholdULTool()
  :m_SensibleMinimumThresholdValue(-100),
  m_SensibleMaximumThresholdValue(+100),
  m_CurrentLowerThresholdValue(1),
  m_CurrentUpperThresholdValue(1)
{
  this->SupportRoiOn();

  m_ThresholdFeedbackNode = DataNode::New();
  m_ThresholdFeedbackNode->SetProperty( "color", ColorProperty::New(1.0, 0.0, 0.0) );
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
  return mitkBinaryThresholdULTool_xpm;
}

const char* mitk::BinaryThresholdULTool::GetName() const
{
  return "ThresholdingUL";
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

void mitk::BinaryThresholdULTool::SetThresholdValues(int lower, int upper)
{
  if (m_ThresholdFeedbackNode.IsNotNull())
  {
    m_CurrentLowerThresholdValue = lower;
    m_CurrentUpperThresholdValue = upper;
    UpdatePreview();
  }
}

void mitk::BinaryThresholdULTool::AcceptCurrentThresholdValue(const std::string& organName, const Color& color)
{

  CreateNewSegmentationFromThreshold(m_NodeForThresholding, organName, color );

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
      // initialize and a new node with the same image as our reference image
      // use the level window property of this image copy to display the result of a thresholding operation
      m_ThresholdFeedbackNode->SetData( image );
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
        m_SensibleMinimumThresholdValue = static_cast<int>( originalImage->GetScalarValueMin() );
        m_SensibleMaximumThresholdValue = static_cast<int>( originalImage->GetScalarValueMax() );
      }

      m_CurrentLowerThresholdValue = (m_SensibleMaximumThresholdValue + m_SensibleMinimumThresholdValue) / 3;
      m_CurrentUpperThresholdValue = 2*m_CurrentLowerThresholdValue;

      IntervalBordersChanged.Send(m_SensibleMinimumThresholdValue, m_SensibleMaximumThresholdValue);
      ThresholdingValuesChanged.Send(m_CurrentLowerThresholdValue, m_CurrentUpperThresholdValue);
    }
  }
}

void mitk::BinaryThresholdULTool::CreateNewSegmentationFromThreshold(DataNode* node, const std::string& organName, const Color& color)
{
  if (node)
  {
    Image::Pointer image = dynamic_cast<Image*>( m_NodeForThresholding->GetData() );
    if (image.IsNotNull())
    {
      // create a new image of the same dimensions and smallest possible pixel type
      DataNode::Pointer emptySegmentation = Tool::CreateEmptySegmentationNode( image, organName, color );

      if (emptySegmentation)
      {
        // actually perform a thresholding and ask for an organ type
        for (unsigned int timeStep = 0; timeStep < image->GetTimeSteps(); ++timeStep)
        {
          try
          {
            ImageTimeSelector::Pointer timeSelector = ImageTimeSelector::New();
            timeSelector->SetInput( image );
            timeSelector->SetTimeNr( timeStep );
            timeSelector->UpdateLargestPossibleRegion();
            Image::Pointer image3D = timeSelector->GetOutput();

            AccessFixedDimensionByItk_2( image3D, ITKThresholding, 3, dynamic_cast<Image*>(emptySegmentation->GetData()), timeStep );
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

        if (DataStorage* ds = m_ToolManager->GetDataStorage())
        {
          ds->Add( emptySegmentation, m_OriginalImageNode );
        }

        m_ToolManager->SetWorkingData( emptySegmentation );
      }
    }
  }
}

template <typename TPixel, unsigned int VImageDimension>
    void mitk::BinaryThresholdULTool::ITKThresholding( itk::Image<TPixel, VImageDimension>* originalImage, Image* segmentation, unsigned int timeStep )
{
  ImageTimeSelector::Pointer timeSelector = ImageTimeSelector::New();
  timeSelector->SetInput( segmentation );
  timeSelector->SetTimeNr( timeStep );
  timeSelector->UpdateLargestPossibleRegion();
  Image::Pointer segmentation3D = timeSelector->GetOutput();

  typedef itk::Image< Tool::DefaultSegmentationDataType, 3> SegmentationType; // this is sure for new segmentations
  SegmentationType::Pointer itkSegmentation;
  CastToItkImage( segmentation3D, itkSegmentation );

  // iterate over original and segmentation
  typedef itk::ImageRegionConstIterator< itk::Image<TPixel, VImageDimension> >     InputIteratorType;
  typedef itk::ImageRegionIterator< SegmentationType >     SegmentationIteratorType;

  InputIteratorType inputIterator( originalImage, originalImage->GetLargestPossibleRegion() );
  SegmentationIteratorType outputIterator( itkSegmentation, itkSegmentation->GetLargestPossibleRegion() );

  inputIterator.GoToBegin();
  outputIterator.GoToBegin();

  while (!outputIterator.IsAtEnd())
  {
    if ( (signed)inputIterator.Get() >= m_CurrentLowerThresholdValue && (signed)inputIterator.Get() <= m_CurrentUpperThresholdValue )
    {
      outputIterator.Set( 1 );
    }
    else
    {
      outputIterator.Set( 0 );
    }

    ++inputIterator;
    ++outputIterator;
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

    m_SensibleMinimumThresholdValue = static_cast<int>( roiFilter->GetMinValue());
    m_SensibleMaximumThresholdValue = static_cast<int>( roiFilter->GetMaxValue());

    m_NodeForThresholding = tmpNode;
  }
  else
    m_NodeForThresholding = m_OriginalImageNode;

  this->SetupPreviewNode();
  this->UpdatePreview();
}

void mitk::BinaryThresholdULTool::UpdatePreview()
{
  typedef itk::Image<int, 3> ImageType;
  typedef itk::Image<unsigned char, 3> SegmentationType;
  typedef itk::BinaryThresholdImageFilter<ImageType, SegmentationType> ThresholdFilterType;
  mitk::Image::Pointer thresholdimage = dynamic_cast<mitk::Image*> (m_NodeForThresholding->GetData());
  if(thresholdimage)
  {
    ImageType::Pointer itkImage = ImageType::New();
    CastToItkImage(thresholdimage, itkImage);
    ThresholdFilterType::Pointer filter = ThresholdFilterType::New();
    filter->SetInput(itkImage);
    filter->SetLowerThreshold(m_CurrentLowerThresholdValue);
    filter->SetUpperThreshold(m_CurrentUpperThresholdValue);
    filter->SetInsideValue(1);
    filter->SetOutsideValue(0);
    filter->Update();

    mitk::Image::Pointer new_image = mitk::Image::New();
    CastToMitkImage(filter->GetOutput(), new_image);
    m_ThresholdFeedbackNode->SetData(new_image);
  }
  RenderingManager::GetInstance()->RequestUpdateAll();
}
