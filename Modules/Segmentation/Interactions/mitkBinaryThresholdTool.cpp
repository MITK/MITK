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

// us
#include "usModule.h"
#include "usModuleResource.h"
#include "usGetModuleContext.h"
#include "usModuleContext.h"

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
  m_PreviewNode = DataNode::New();
  mitk::CoreObjectFactory::GetInstance()->SetDefaultProperties( m_PreviewNode );

  m_PreviewNode->SetProperty( "color", ColorProperty::New(0.0, 1.0, 0.0) );
  m_PreviewNode->SetProperty( "texture interpolation", BoolProperty::New(false) );
  m_PreviewNode->SetProperty( "layer", IntProperty::New( 100 ) );
  m_PreviewNode->SetProperty( "levelwindow", LevelWindowProperty::New( LevelWindow(100, 1) ) );
  m_PreviewNode->SetProperty( "name", StringProperty::New("BinaryThresholdTool preview") );
  m_PreviewNode->SetProperty( "opacity", FloatProperty::New(0.3) );
  m_PreviewNode->SetProperty( "helper object", BoolProperty::New(true) );
}

mitk::BinaryThresholdTool::~BinaryThresholdTool()
{
}

const char** mitk::BinaryThresholdTool::GetXPM() const
{
  return NULL;
}

us::ModuleResource mitk::BinaryThresholdTool::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  assert(module);
  us::ModuleResource resource = module->GetResource("Threshold_48x48.png");
  return resource;
}

const char* mitk::BinaryThresholdTool::GetName() const
{
  return "Threshold";
}

void mitk::BinaryThresholdTool::Activated()
{
  m_ReferenceNode = m_ToolManager->GetReferenceData(0);
  assert(m_ReferenceNode.IsNotNull());

  m_NodeForThresholding = m_ReferenceNode;
  m_CurrentTimeStep = 0; // todo: get the right time step e.g. from GUI
  this->SetupPreviewNodeFor( m_NodeForThresholding );
  RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::BinaryThresholdTool::Deactivated()
{
  m_NodeForThresholding = NULL;
  m_ReferenceNode = NULL;
  m_ToolManager->GetDataStorage()->Remove( m_PreviewNode );
  m_PreviewNode->SetData(NULL);
//  mitk::UndoController::GetCurrentUndoModel()->Clear();
//  mitk::UndoController::GetCurrentUndoModel()->ClearRedoList();
}

void mitk::BinaryThresholdTool::SetThresholdValue(double value)
{
  if (value != m_CurrentThresholdValue)
  {
    m_CurrentThresholdValue = value;
    m_PreviewNode->SetProperty( "levelwindow", LevelWindowProperty::New( LevelWindow(m_CurrentThresholdValue, 0.001) ) );
    RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void mitk::BinaryThresholdTool::AcceptPreview()
{
  Image* referenceImage = dynamic_cast<Image*>( m_ToolManager->GetReferenceData(0)->GetData() );
  assert(referenceImage);

  LabelSetImage* workingImage = dynamic_cast<LabelSetImage*>( m_ToolManager->GetWorkingData(0)->GetData() );
  assert(workingImage);

  CurrentlyBusy.Send(true);

  try
  {
//    this->InitializeUndoController();

    // perform the actual thresholding
    ImageTimeSelector::Pointer timeSelector = ImageTimeSelector::New();
    timeSelector->SetInput( workingImage );
    timeSelector->SetTimeNr( m_CurrentTimeStep );
    timeSelector->UpdateLargestPossibleRegion();
    Image::Pointer workingImageTimeStep = timeSelector->GetOutput();

    if (workingImageTimeStep->GetDimension() == 2)
    {
      AccessTwoImagesFixedDimensionByItk( workingImageTimeStep, referenceImage, ITKThresholding, 2 );
    }
    else
    {
      AccessTwoImagesFixedDimensionByItk( workingImageTimeStep, referenceImage, ITKThresholding, 3 );
    }

    workingImageTimeStep->Modified();
    workingImage->Modified();
  }
  catch (itk::ExceptionObject& e)
  {
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    Tool::ErrorMessage("Could not create segmentation.");
  }
  catch (...)
  {
    MITK_ERROR << "Unknown exception!";
    Tool::ErrorMessage("Could not create segmentation.");
  }

  CurrentlyBusy.Send(false);

  RenderingManager::GetInstance()->RequestUpdateAll();
  m_ToolManager->ActivateTool(-1);
}

void mitk::BinaryThresholdTool::CancelThresholding()
{
  m_ToolManager->ActivateTool(-1);
}

void mitk::BinaryThresholdTool::SetupPreviewNodeFor( DataNode* nodeForThresholding )
{
  if (!nodeForThresholding) return;

  unsigned int timestep = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetTime()->GetPos();

  Image::Pointer imageForThresholding = dynamic_cast<Image*>( nodeForThresholding->GetData() );
  Image::Pointer imageForThresholdingTimeStep = this->Get3DImage(imageForThresholding, timestep);

  Image::Pointer originalImage = dynamic_cast<Image*> (m_ReferenceNode->GetData());
  Image::Pointer originalImageTimeStep = this->Get3DImage(originalImage, timestep);
  if (originalImageTimeStep.IsNotNull() && imageForThresholding.IsNotNull())
  {
    // initialize a new node with the same image as our reference image
    // use the level window property of this image copy to display the result of a thresholding operation
    m_PreviewNode->SetData( imageForThresholding );
    int layer(50);
    nodeForThresholding->GetIntProperty("layer", layer);
    m_PreviewNode->SetIntProperty("layer", layer+1);

    if (DataStorage* storage = m_ToolManager->GetDataStorage())
    {
      if (storage->Exists(m_PreviewNode))
         storage->Remove(m_PreviewNode);

      storage->Add( m_PreviewNode);
    }

    if (imageForThresholding.GetPointer() == originalImageTimeStep.GetPointer())
    {
      if ((originalImageTimeStep->GetPixelType().GetPixelType() == itk::ImageIOBase::SCALAR)
        &&(originalImageTimeStep->GetPixelType().GetComponentType() == itk::ImageIOBase::FLOAT))
         m_IsFloatImage = true;
      else
         m_IsFloatImage = false;

     m_SensibleMinimumThresholdValue = static_cast<double>( originalImageTimeStep->GetScalarValueMin() );
     m_SensibleMaximumThresholdValue = static_cast<double>( originalImageTimeStep->GetScalarValueMax() );
    }

    LevelWindowProperty::Pointer lwp = dynamic_cast<LevelWindowProperty*>( m_PreviewNode->GetProperty( "levelwindow" ));
    if ( !m_IsFloatImage )
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

template <typename TPixel1, unsigned int VDimension1, typename TPixel2, unsigned int VDimension2>
void mitk::BinaryThresholdTool::ITKThresholding( itk::Image<TPixel1, VDimension1>* targetImage,
                                                 const itk::Image<TPixel2, VDimension2>* sourceImage )
{
  typedef typename itk::Image< TPixel1, VDimension1> TargetImageType;
  typedef typename itk::Image< TPixel2, VDimension2> SourceImageType;

  typedef itk::ImageRegionConstIterator< SourceImageType > SourceIteratorType;
  typedef typename itk::ImageRegionIterator< TargetImageType > TargetIteratorType;

  SourceIteratorType sourceIterator( sourceImage, sourceImage->GetLargestPossibleRegion() );
  TargetIteratorType targetIterator( targetImage, targetImage->GetLargestPossibleRegion() );

  LabelSetImage* workingImage = dynamic_cast<LabelSetImage*>(m_ToolManager->GetWorkingData(0)->GetData());
  assert(workingImage);

  int activePixelValue = workingImage->GetActiveLabel()->GetIndex();

  sourceIterator.GoToBegin();
  targetIterator.GoToBegin();

  while (!targetIterator.IsAtEnd())
  {
    int targetValue = static_cast<int>( targetIterator.Get() );
    if ( sourceIterator.Get() >= m_CurrentThresholdValue )
    {
      if (!workingImage->GetLabelLocked(targetValue))
      {
        targetIterator.Set(activePixelValue);
      }
    }
    ++sourceIterator;
    ++targetIterator;
  }
}
