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
#include "mitkLevelWindowProperty.h"
#include "mitkProperties.h"
#include "mitkDataStorage.h"
#include "mitkRenderingManager.h"
#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageTimeSelector.h"
#include "mitkLabelSetImage.h"
#include "mitkSliceNavigationController.h"

// us
#include "usModule.h"
#include "usModuleResource.h"
#include "usGetModuleContext.h"
#include "usModuleContext.h"

//itk
#include <itkImageRegionIterator.h>

namespace mitk {
  MITK_TOOL_MACRO(MitkSegmentation_EXPORT, BinaryThresholdTool, "Thresholding tool");
}

mitk::BinaryThresholdTool::BinaryThresholdTool() : SegTool3D("dummy"),
m_SensibleMinimumThresholdValue(-100),
m_SensibleMaximumThresholdValue(+100),
m_CurrentThresholdValue(0.0),
m_IsFloatImage(false)
{

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
  Superclass::Activated();

  bool referenceHasChanged(m_ReferenceNode != m_ToolManager->GetReferenceData(0));

  m_ReferenceNode = m_ToolManager->GetReferenceData(0);
  assert(m_ReferenceNode);

  m_NodeForThresholding = m_ReferenceNode;

  m_CurrentTimeStep = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetTime()->GetPos();

  Image::Pointer imageForThresholding = dynamic_cast<Image*>( m_NodeForThresholding->GetData() );
  Image::Pointer imageForThresholdingTimeStep = this->Get3DImage(imageForThresholding, m_CurrentTimeStep);

  Image::Pointer originalImage = dynamic_cast<Image*> (m_ReferenceNode->GetData());
  Image::Pointer originalImageTimeStep = this->Get3DImage(originalImage, m_CurrentTimeStep);

  // initialize a new node with the same image as the reference image
  m_PreviewNode->SetData( imageForThresholdingTimeStep );

  if ((originalImageTimeStep->GetPixelType().GetPixelType() == itk::ImageIOBase::SCALAR)
    && (originalImageTimeStep->GetPixelType().GetComponentType() == itk::ImageIOBase::FLOAT))
      m_IsFloatImage = true;
  else
      m_IsFloatImage = false;

  if (referenceHasChanged)
  {
    CurrentlyBusy.Send(true);

    m_SensibleMinimumThresholdValue = static_cast<mitk::ScalarType>( originalImageTimeStep->GetScalarValueMin() );
    m_SensibleMaximumThresholdValue = static_cast<mitk::ScalarType>( originalImageTimeStep->GetScalarValueMaxNoRecompute() );
    m_CurrentThresholdValue = (m_SensibleMaximumThresholdValue + m_SensibleMinimumThresholdValue)/2;

    CurrentlyBusy.Send(false);
  }

  m_PreviewNode->SetProperty( "levelwindow", LevelWindowProperty::New( LevelWindow(m_CurrentThresholdValue, 0.001) ) );
  m_PreviewNode->SetProperty( "binary", BoolProperty::New(false) );
  m_PreviewNode->SetOpacity(0.6);

  IntervalBordersChanged.Send(m_SensibleMinimumThresholdValue, m_SensibleMaximumThresholdValue, m_IsFloatImage);
  ThresholdingValueChanged.Send(m_CurrentThresholdValue);

  RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::BinaryThresholdTool::Deactivated()
{
  SegTool3D::Deactivated();
  m_NodeForThresholding = NULL;
//  mitk::UndoController::GetCurrentUndoModel()->Clear();
//  mitk::UndoController::GetCurrentUndoModel()->ClearRedoList();
}

void mitk::BinaryThresholdTool::SetThresholdValue(double value)
{
  if (value != m_CurrentThresholdValue)
  {
    m_CurrentThresholdValue = value;
    m_PreviewNode->SetProperty( "levelwindow", LevelWindowProperty::New( LevelWindow(m_CurrentThresholdValue, 0.001) ) );
    m_PreviewNode->SetVisibility(true);
    RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void mitk::BinaryThresholdTool::CreateNewLabel(const std::string& name, const mitk::Color& color)
{
  if ( m_PreviewNode.IsNull() ) return;

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(m_WorkingNode->GetData());
  assert(workingImage);

  workingImage->AddLabel(name,color);

  this->AcceptPreview();
}

void mitk::BinaryThresholdTool::AcceptPreview()
{
  Image* referenceImage = dynamic_cast<Image*>( m_ReferenceNode->GetData() );
  assert(referenceImage);

  LabelSetImage* workingImage = dynamic_cast<LabelSetImage*>( m_WorkingNode->GetData() );
  assert(workingImage);

  m_PaintingPixelValue = workingImage->GetActiveLabelPixelValue();

  CurrentlyBusy.Send(true);

  try
  {
//    this->InitializeUndoController();
    Image::Pointer workingImageTimeStep = this->Get3DImage(workingImage, m_CurrentTimeStep);

    if (workingImageTimeStep->GetDimension() == 2)
    {
      AccessTwoImagesFixedDimensionByItk( workingImageTimeStep, referenceImage, InternalAcceptPreview, 2 );
    }
    else
    {
      AccessTwoImagesFixedDimensionByItk( workingImageTimeStep, referenceImage, InternalAcceptPreview, 3 );
    }

    workingImageTimeStep->Modified();
    workingImage->Modified();
  }
  catch (itk::ExceptionObject& e)
  {
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    Tool::ErrorMessage("Could not create segmentation. See error log for details.");
  }
  catch (...)
  {
    MITK_ERROR << "Unknown exception!";
    Tool::ErrorMessage("Could not create segmentation.");
  }

  CurrentlyBusy.Send(false);

  m_PreviewNode->SetVisibility(false);

  RenderingManager::GetInstance()->RequestUpdateAll();
}

template <typename TPixel1, unsigned int VDimension1, typename TPixel2, unsigned int VDimension2>
void mitk::BinaryThresholdTool::InternalAcceptPreview( itk::Image<TPixel1, VDimension1>* targetImage,
                                                 const itk::Image<TPixel2, VDimension2>* sourceImage )
{
  typedef typename itk::Image< TPixel1, VDimension1> TargetImageType;
  typedef typename itk::Image< TPixel2, VDimension2> SourceImageType;

  typedef itk::ImageRegionConstIterator< SourceImageType > SourceIteratorType;
  typedef typename itk::ImageRegionIterator< TargetImageType > TargetIteratorType;

  SourceIteratorType sourceIterator( sourceImage, sourceImage->GetLargestPossibleRegion() );
  TargetIteratorType targetIterator( targetImage, targetImage->GetLargestPossibleRegion() );

  LabelSetImage* workingImage = dynamic_cast<LabelSetImage*>(m_WorkingNode->GetData());
  assert(workingImage);

  int activePixelValue = workingImage->GetActiveLabel()->GetPixelValue();

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
