/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
Version:   $Revision: 1.12 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkBinaryThresholdULTool.h"

#include "mitkBinaryThresholdULTool.xpm"

#include "mitkToolManager.h"
#include "mitkBoundingObjectToSegmentationFilter.h"

#include <mitkCoreObjectFactory.h>
#include "mitkLevelWindowProperty.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkOrganTypeProperty.h"
#include "mitkVtkResliceInterpolationProperty.h"
#include "mitkDataStorage.h"
#include "mitkRenderingManager.h"

#include "mitkImageCast.h"
#include "mitkImageTimeSelector.h"
#include <itkImageRegionIterator.h>
#include <itkMaskImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>

namespace mitk {
  MITK_TOOL_MACRO(MitkExt_EXPORT, BinaryThresholdULTool, "ThresholdingUL tool");
}

mitk::BinaryThresholdULTool::BinaryThresholdULTool()
:m_SensibleMinimumThresholdValue(-100),
m_SensibleMaximumThresholdValue(+100),
m_CurrentLowerThresholdValue(1),
m_CurrentUpperThresholdValue(1)
{
  this->SupportRoiOn();

  m_ThresholdFeedbackNode = DataNode::New();
  mitk::CoreObjectFactory::GetInstance()->SetDefaultProperties( m_ThresholdFeedbackNode );

  m_ThresholdFeedbackNode->SetProperty( "color", ColorProperty::New(1.0, 0.0, 0.0) );
  m_ThresholdFeedbackNode->SetProperty( "texture interpolation", BoolProperty::New(false) );
  m_ThresholdFeedbackNode->SetProperty( "layer", IntProperty::New( 100 ) );
  //m_ThresholdFeedbackNode->SetProperty( "levelwindow", LevelWindowProperty::New( LevelWindow(100, 1) ) );
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
    SetupPreviewNodeFor( m_NodeForThresholding );
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
    //m_ThresholdFeedbackNode->SetProperty( "levelwindow", LevelWindowProperty::New( LevelWindow(m_CurrentThresholdValue, 1) ) );
    RenderingManager::GetInstance()->RequestUpdateAll();
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

void mitk::BinaryThresholdULTool::SetupPreviewNodeFor( DataNode* nodeForThresholding )
{
  if (nodeForThresholding)
  {
    Image::Pointer image = dynamic_cast<Image*>( nodeForThresholding->GetData() );
    Image::Pointer originalImage = dynamic_cast<Image*> (m_OriginalImageNode->GetData());
    if (image.IsNotNull())
    {
      // initialize and a new node with the same image as our reference image
      // use the level window property of this image copy to display the result of a thresholding operation
      m_ThresholdFeedbackNode->SetData( image );
      int layer(50);
      nodeForThresholding->GetIntProperty("layer", layer);
      m_ThresholdFeedbackNode->SetIntProperty("layer", layer+1);

      if (DataStorage* storage = m_ToolManager->GetDataStorage())
      {
        if (storage->Exists(m_ThresholdFeedbackNode))
          storage->Remove(m_ThresholdFeedbackNode);
        storage->Add( m_ThresholdFeedbackNode, nodeForThresholding );
      }

      m_SensibleMinimumThresholdValue = static_cast<int>( originalImage->GetScalarValueMin() );
      m_SensibleMaximumThresholdValue = static_cast<int>( originalImage->GetScalarValueMax() );

     /* LevelWindowProperty::Pointer lwp = dynamic_cast<LevelWindowProperty*>( m_ThresholdFeedbackNode->GetProperty( "levelwindow" ));
      if (lwp)
      {
        m_CurrentLowerThresholdValue = static_cast<int>( lwp->GetLevelWindow().GetLevel() );
        m_CurrentUpperThresholdValue = m_CurrentLowerThresholdValue+100;
      }
      else
      {*/
        m_CurrentLowerThresholdValue = (m_SensibleMaximumThresholdValue + m_SensibleMinimumThresholdValue) / 3;
        m_CurrentUpperThresholdValue = 2*m_CurrentLowerThresholdValue;
      //}

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

        if (DataStorage* storage = m_ToolManager->GetDataStorage())
        {
          storage->Add( emptySegmentation, m_OriginalImageNode ); // add as a child, because the segmentation "derives" from the original
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
  typedef itk::Image<int, 3> ItkImageType;
  typedef itk::Image<unsigned char, 3> ItkMaskType;
  typedef itk::MaskImageFilter<ItkImageType, ItkMaskType, ItkImageType> MaskFilterType;
  mitk::DataNode* node = m_ToolManager->GetRoiData(0);
  if (node == NULL)
  {
    this->SetupPreviewNodeFor(m_OriginalImageNode);
    m_NodeForThresholding = m_OriginalImageNode;
    return;
  }

  mitk::DataNode::Pointer new_node = mitk::DataNode::New();
  mitk::Image* image = dynamic_cast<mitk::Image*> (m_OriginalImageNode->GetData());
  mitk::Image::Pointer new_image = mitk::Image::New();

  mitk::Image::Pointer roi;
  mitk::BoundingObject* boundingObject = dynamic_cast<mitk::BoundingObject*> (node->GetData());

  if (boundingObject)
  {
    mitk::BoundingObjectToSegmentationFilter::Pointer filter = mitk::BoundingObjectToSegmentationFilter::New();
    filter->SetBoundingObject( boundingObject);
    filter->SetInput(image);
    filter->Update();
    roi = filter->GetOutput();
  }
  else
    roi =  dynamic_cast<mitk::Image*> (node->GetData());

  if (roi)
  {
    MaskFilterType::Pointer filter = MaskFilterType::New();
    ItkMaskType::Pointer itkRoi = ItkMaskType::New();
    ItkImageType::Pointer itkImage = ItkImageType::New();
    mitk::CastToItkImage(image, itkImage);
    mitk::CastToItkImage(roi, itkRoi);
    filter->SetInput1(itkImage);
    filter->SetInput2(itkRoi);
    filter->SetOutsideValue(-32765);
    filter->Update();
    mitk::CastToMitkImage(filter->GetOutput(),new_image);
  }
  new_node->SetData(new_image);

  this->SetupPreviewNodeFor(new_node);
  m_NodeForThresholding = new_node;
  UpdatePreview();
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
}