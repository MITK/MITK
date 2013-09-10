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

#include "mitkBinaryThresholdTool.h"

#include "mitkBinaryThresholdTool.xpm"

#include "mitkToolManager.h"
#include "mitkCoreObjectFactory.h"
#include "mitkLevelWindowProperty.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkDataStorage.h"
#include "mitkRenderingManager.h"
#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageTimeSelector.h"
#include "mitkPadImageFilter.h"
#include "mitkLabelSetImage.h"
#include "mitkMaskAndCutRoiImageFilter.h"

// us
#include "mitkModule.h"
#include "mitkModuleResource.h"
#include "mitkGetModuleContext.h"

//itk
#include <itkImageRegionIterator.h>

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, BinaryThresholdTool, "Thresholding tool");
}

mitk::BinaryThresholdTool::BinaryThresholdTool()
:m_SensibleMinimumThresholdValue(-100),
m_SensibleMaximumThresholdValue(+100),
m_CurrentThresholdValue(0.0),
m_IsFloatImage(false)
{
  this->SupportRoiOn();

  m_ThresholdFeedbackNode = DataNode::New();
  mitk::CoreObjectFactory::GetInstance()->SetDefaultProperties( m_ThresholdFeedbackNode );

  m_ThresholdFeedbackNode->SetProperty( "color", ColorProperty::New(0.0, 1.0, 0.0) );
  m_ThresholdFeedbackNode->SetProperty( "texture interpolation", BoolProperty::New(false) );
  m_ThresholdFeedbackNode->SetProperty( "layer", IntProperty::New( 100 ) );
  m_ThresholdFeedbackNode->SetProperty( "levelwindow", LevelWindowProperty::New( LevelWindow(100, 1) ) );
  m_ThresholdFeedbackNode->SetProperty( "name", StringProperty::New("Thresholding feedback") );
  m_ThresholdFeedbackNode->SetProperty( "opacity", FloatProperty::New(0.3) );
  m_ThresholdFeedbackNode->SetProperty( "helper object", BoolProperty::New(false) );
}

mitk::BinaryThresholdTool::~BinaryThresholdTool()
{
}

const char** mitk::BinaryThresholdTool::GetXPM() const
{
  return NULL;
}

mitk::ModuleResource mitk::BinaryThresholdTool::GetIconResource() const
{
  Module* module = GetModuleContext()->GetModule();
  ModuleResource resource = module->GetResource("Threshold_48x48.png");
  return resource;
}

const char* mitk::BinaryThresholdTool::GetName() const
{
  return "Threshold";
}

void mitk::BinaryThresholdTool::Activated()
{
  m_ToolManager->RoiDataChanged += mitk::MessageDelegate<mitk::BinaryThresholdTool>(this, &mitk::BinaryThresholdTool::OnRoiDataChanged);
  m_ReferenceNode = m_ToolManager->GetReferenceData(0);
  m_NodeForThresholding = m_ReferenceNode;

  if ( m_NodeForThresholding.IsNotNull() )
  {
    this->SetupPreviewNodeFor( m_NodeForThresholding );
  }
  else
  {
    m_ToolManager->ActivateTool(-1);
  }
}

void mitk::BinaryThresholdTool::Deactivated()
{
  m_ToolManager->RoiDataChanged -= mitk::MessageDelegate<mitk::BinaryThresholdTool>(this, &mitk::BinaryThresholdTool::OnRoiDataChanged);
  m_NodeForThresholding = NULL;
  m_ReferenceNode = NULL;
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

void mitk::BinaryThresholdTool::SetThresholdValue(double value)
{
  if (m_ThresholdFeedbackNode.IsNotNull())
  {
    m_CurrentThresholdValue = value;
    m_ThresholdFeedbackNode->SetProperty( "levelwindow", LevelWindowProperty::New( LevelWindow(m_CurrentThresholdValue, 0.001) ) );
    RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void mitk::BinaryThresholdTool::ConfirmSegmentation()
{
    Image::Pointer thresholdImage = dynamic_cast<Image*>( m_NodeForThresholding->GetData() );
    if (thresholdImage.IsNull()) return;

    LabelSetImage::Pointer workingImage = dynamic_cast<LabelSetImage*>( m_ToolManager->GetWorkingData(0)->GetData() );
    if (workingImage.IsNull()) return;

    CurrentlyBusy.Send(true);

    // perform the actual thresholding
    for (unsigned int timeStep = 0; timeStep < workingImage->GetTimeSteps(); ++timeStep)
    {
        ImageTimeSelector::Pointer timeSelector = ImageTimeSelector::New();
        timeSelector->SetInput( workingImage );
        timeSelector->SetTimeNr( timeStep );
        timeSelector->UpdateLargestPossibleRegion();
        Image::Pointer workingImageTimeStep = timeSelector->GetOutput();

        try
        {
          if (workingImageTimeStep->GetDimension() == 2)
          {
            AccessFixedDimensionByItk_2( workingImageTimeStep, ITKThresholding, 2,thresholdImage, timeStep );
          }
          else
          {
            AccessFixedDimensionByItk_2( workingImageTimeStep, ITKThresholding, 3, thresholdImage, timeStep );
          }
        }
        catch(...)
        {
          CurrentlyBusy.Send(false);
          Tool::ErrorMessage("Error accessing single time steps of the original image. Cannot create segmentation.");
        }
    }

    CurrentlyBusy.Send(false);

    workingImage->Modified();
    workingImage->GetVtkImageData()->Modified();

    RenderingManager::GetInstance()->RequestUpdateAll();
    m_ToolManager->ActivateTool(-1);
}

void mitk::BinaryThresholdTool::CancelThresholding()
{
  m_ToolManager->ActivateTool(-1);
}

void mitk::BinaryThresholdTool::SetupPreviewNodeFor( DataNode* nodeForThresholding )
{
  if (nodeForThresholding)
  {
    Image::Pointer image = dynamic_cast<Image*>( nodeForThresholding->GetData() );
    Image::Pointer originalImage = dynamic_cast<Image*> (m_ReferenceNode->GetData());
    if (image.IsNotNull())
    {
      // initialize a new node with the same image as our reference image
      // use the level window property of this image copy to display the result of a thresholding operation
      m_ThresholdFeedbackNode->SetData( image );
      int layer(50);
      nodeForThresholding->GetIntProperty("layer", layer);
      m_ThresholdFeedbackNode->SetIntProperty("layer", layer+1);

      if (DataStorage* storage = m_ToolManager->GetDataStorage())
      {
        if (storage->Exists(m_ThresholdFeedbackNode))
           storage->Remove(m_ThresholdFeedbackNode);

        storage->Add( m_ThresholdFeedbackNode);
      }

      if (image.GetPointer() == originalImage.GetPointer())
      {
        if ((originalImage->GetPixelType().GetPixelType() == itk::ImageIOBase::SCALAR)
          &&(originalImage->GetPixelType().GetComponentType() == itk::ImageIOBase::FLOAT))
           m_IsFloatImage = true;
        else
           m_IsFloatImage = false;

       m_SensibleMinimumThresholdValue = static_cast<double>( originalImage->GetScalarValueMin() );
       m_SensibleMaximumThresholdValue = static_cast<double>( originalImage->GetScalarValueMax() );
      }

      LevelWindowProperty::Pointer lwp = dynamic_cast<LevelWindowProperty*>( m_ThresholdFeedbackNode->GetProperty( "levelwindow" ));
      if (lwp && !m_IsFloatImage )
      {
        m_CurrentThresholdValue = static_cast<double>( lwp->GetLevelWindow().GetLevel() );
      }
      else
      {
        m_CurrentThresholdValue = (m_SensibleMaximumThresholdValue + m_SensibleMinimumThresholdValue)/2;
      }

      IntervalBordersChanged.Send(m_SensibleMinimumThresholdValue, m_SensibleMaximumThresholdValue, m_IsFloatImage);
      ThresholdingValueChanged.Send(m_CurrentThresholdValue);
    }
  }
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::BinaryThresholdTool::ITKThresholding( itk::Image<TPixel, VImageDimension>* targetImage, Image* sourceImage, unsigned int timeStep )
{
  typedef itk::Image<TPixel,VImageDimension> ImageType;

  typename ImageType::Pointer sourceImageITK;
  CastToItkImage( sourceImage, sourceImageITK );

  typedef itk::ImageRegionConstIterator< ImageType > SourceIteratorType;
  typedef itk::ImageRegionIterator< ImageType >      TargetIteratorType;

  typename SourceIteratorType sourceIterator( sourceImageITK, sourceImageITK->GetLargestPossibleRegion() );
  typename TargetIteratorType targetIterator( targetImage, targetImage->GetLargestPossibleRegion() );

  sourceIterator.GoToBegin();
  targetIterator.GoToBegin();

  const int& activePixelValue = m_ToolManager->GetActiveLabel()->GetIndex();

  while (!targetIterator.IsAtEnd())
  {
    const int targetValue = targetIterator.Get();
    if ( sourceIterator.Get() >= m_CurrentThresholdValue )
    {
      if (!m_ToolManager->GetLabelLocked(targetValue))
        targetIterator.Set( activePixelValue );
    }

    ++sourceIterator;
    ++targetIterator;
  }
}

void mitk::BinaryThresholdTool::OnRoiDataChanged()
{
  mitk::DataNode::Pointer roiNode = m_ToolManager->GetRoiData(0);

  if (roiNode.IsNotNull())
  {
    mitk::Image::Pointer thresholdImage = dynamic_cast<mitk::Image*> (m_NodeForThresholding->GetData());

    if (thresholdImage.IsNull())
      return;

    mitk::MaskAndCutRoiImageFilter::Pointer roiFilter = mitk::MaskAndCutRoiImageFilter::New();

    roiFilter->SetInput(thresholdImage);
    roiFilter->SetRegionOfInterest(roiNode->GetData());
    roiFilter->Update();

    mitk::DataNode::Pointer tmpNode = mitk::DataNode::New();
    mitk::Image::Pointer tmpImage = roiFilter->GetOutput();

    tmpNode->SetData(tmpImage);

    m_SensibleMaximumThresholdValue = static_cast<int> (roiFilter->GetMaxValue());
    m_SensibleMinimumThresholdValue = static_cast<int> (roiFilter->GetMinValue());
    SetupPreviewNodeFor( tmpNode );
    m_NodeForThresholding = tmpNode;

    return;
  }
  else
  {
    this->SetupPreviewNodeFor(m_ReferenceNode);
    m_NodeForThresholding = m_ReferenceNode;
    return;
  }
}
