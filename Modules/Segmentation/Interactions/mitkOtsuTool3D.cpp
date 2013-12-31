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

#include "mitkOtsuTool3D.h"

// mitk
#include "mitkLabelSetImage.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
#include "mitkInteractionConst.h"
#include "mitkImageAccessByItk.h"
#include "mitkToolManager.h"
#include "mitkImageCast.h"
#include "mitkColormapProperty.h"
#include "mitkRenderingModeProperty.h"
#include "mitkLevelWindowProperty.h"

// itk
#include <itkOtsuMultipleThresholdsImageFilter.h>
#include <itkBinaryImageToShapeLabelMapFilter.h>
#include <itkRelabelLabelMapFilter.h>
#include <itkLabelMapToLabelImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, OtsuTool3D, "Otsu Segmentation");
}

mitk::OtsuTool3D::OtsuTool3D() : SegTool3D("dummy"), m_NumberOfRegions(3), m_SelectedRegion(0)
{
}

mitk::OtsuTool3D::~OtsuTool3D()
{

}

void mitk::OtsuTool3D::SetNumberOfRegions(int value)
{
  if (value>1 && value<5)
    m_NumberOfRegions = value;
}

int mitk::OtsuTool3D::GetNumberOfRegions()
{
  return m_NumberOfRegions;
}
/*
void mitk::OtsuTool3D::AcceptPreview(int region)
{
  m_SelectedRegion = region;

  mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( m_WorkingNode->GetData() );
  assert(workingImage);

  CurrentlyBusy.Send(true);

  try
  {
    AccessTwoImagesFixedDimensionByItk( workingImage, m_PreviewImage, InternalAcceptPreview, 3);
  }
  catch( itk::ExceptionObject & e )
  {
    CurrentlyBusy.Send(false);
    m_ProgressCommand->Reset();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    m_ToolManager->ActivateTool(-1);
    return;
  }
  catch (...)
  {
    CurrentlyBusy.Send(false);
    m_ProgressCommand->Reset();
    MITK_ERROR << "Unkown exception caught!";
    m_ToolManager->ActivateTool(-1);
    return;
  }

  workingImage->Modified();

  CurrentlyBusy.Send(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
*/
void mitk::OtsuTool3D::UpdatePreview(int region)
{
/*
  if (region<0)
  {
    m_PreviewNode->SetVisibility(false);
    return;
  }
*/
  mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( m_WorkingNode->GetData() );
  assert(workingImage);

  CurrentlyBusy.Send(true);

  try
  {
    AccessByItk_1( m_MultiLabelImage, InternalUpdatePreview, region);
  }
  catch( itk::ExceptionObject & e )
  {
    CurrentlyBusy.Send(false);
    m_ProgressCommand->Reset();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    m_ToolManager->ActivateTool(-1);
    return;
  }
  catch (...)
  {
    CurrentlyBusy.Send(false);
    m_ProgressCommand->Reset();
    MITK_ERROR << "Unkown exception caught!";
    m_ToolManager->ActivateTool(-1);
    return;
  }

  m_PreviewNode->SetVisibility(true);

  workingImage->Modified();

  CurrentlyBusy.Send(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

const char** mitk::OtsuTool3D::GetXPM() const
{
  return NULL;
}

us::ModuleResource mitk::OtsuTool3D::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Otsu_48x48.png");
  return resource;
}

void mitk::OtsuTool3D::Activated()
{
  Superclass::Activated();

  // feedback node and its visualization properties
  m_MultiLabelNode = mitk::DataNode::New();
  m_MultiLabelNode->SetName( "multilabel preview" );
  m_MultiLabelNode->SetOpacity(0.7);
  m_MultiLabelNode->SetColor(1.0, 1.0, 1.0);
  m_MultiLabelNode->SetProperty( "Image Rendering.Mode", RenderingModeProperty::New( RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW) );
  m_MultiLabelNode->SetProperty( "colormap", ColormapProperty::New(ColormapProperty::CM_MULTILABEL) );
  m_MultiLabelNode->SetProperty( "texture interpolation", BoolProperty::New(false) );
  m_MultiLabelNode->SetProperty( "layer", IntProperty::New(100) );
  m_MultiLabelNode->SetProperty( "binary", BoolProperty::New(false) );
  m_MultiLabelNode->SetProperty( "helper object", BoolProperty::New(true) );

  mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
  mitk::LevelWindow levelwindow;
  levelwindow.SetLevelWindow(127.5, 255);
  levelwindow.SetRangeMinMax(0, 255);
  levWinProp->SetLevelWindow(levelwindow);
  m_MultiLabelNode->SetProperty( "levelwindow", levWinProp );

  m_ToolManager->GetDataStorage()->Add(m_MultiLabelNode, m_WorkingNode);

  m_PreviewNode->SetProperty("outline binary", BoolProperty::New(false) );
  m_PreviewNode->SetOpacity(0.6);
  m_PreviewNode->SetColor(1.0, 0.0, 0.0);
}

void mitk::OtsuTool3D::Deactivated()
{
  Superclass::Deactivated();

  m_ToolManager->GetDataStorage()->Remove( m_MultiLabelNode );
  m_MultiLabelNode = NULL;
  m_MultiLabelImage = NULL;
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::OtsuTool3D::Run()
{
  m_ReferenceNode = m_ToolManager->GetReferenceData(0);
  assert(m_ReferenceNode);

  mitk::Image* referenceImage = dynamic_cast< mitk::Image* >( m_ReferenceNode->GetData() );
  assert(referenceImage);

  mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( m_WorkingNode->GetData() );
  assert(workingImage);

  m_PaintingPixelValue = workingImage->GetActiveLabelIndex();

  m_CurrentTimeStep = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetTime()->GetPos();

  //  this->InitializeUndoController();
  CurrentlyBusy.Send(true);

  try
  {
    AccessByItk(referenceImage, InternalRun);
  }
  catch( itk::ExceptionObject & e )
  {
    CurrentlyBusy.Send(false);
    m_ProgressCommand->Reset();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    m_ToolManager->ActivateTool(-1);
    return;
  }
  catch (...)
  {
    CurrentlyBusy.Send(false);
    m_ProgressCommand->Reset();
    MITK_ERROR << "Unkown exception caught!";
    m_ToolManager->ActivateTool(-1);
    return;
  }

  CurrentlyBusy.Send(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

const char* mitk::OtsuTool3D::GetName() const
{
  return "Otsu";
}

template <typename ImageType>
void mitk::OtsuTool3D::InternalUpdatePreview(const ImageType* input, int region)
{
  typedef itk::BinaryThresholdImageFilter< ImageType, ImageType > ThresholdFilterType;

  typename ThresholdFilterType::Pointer thresholdFilter = ThresholdFilterType::New();
  thresholdFilter->SetInput(input);
  thresholdFilter->SetLowerThreshold(region);
  thresholdFilter->SetUpperThreshold(region);
  thresholdFilter->SetOutsideValue(0);
  thresholdFilter->SetInsideValue(1);

  if (m_ProgressCommand.IsNotNull())
  {
    m_ProgressCommand->AddStepsToDo(200);
    thresholdFilter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);
  }

  thresholdFilter->Update();

  if (m_ProgressCommand.IsNotNull())
  {
    m_ProgressCommand->Reset();
  }

  typename ImageType::Pointer result = thresholdFilter->GetOutput();
  result->DisconnectPipeline();

  m_PreviewImage = mitk::Image::New();
  mitk::CastToMitkImage(result.GetPointer(), m_PreviewImage);

  m_PreviewNode->SetData(m_PreviewImage);

  m_RequestedRegion = m_PreviewImage->GetLargestPossibleRegion();
}

template < typename TPixel, unsigned int VDimension >
void mitk::OtsuTool3D::InternalRun( itk::Image<TPixel, VDimension>* input )
{
  typedef itk::Image< TPixel, VDimension > InputImageType;
  typedef itk::Image< LabelSetImage::PixelType, VDimension > OutputImageType;
  typedef itk::OtsuMultipleThresholdsImageFilter< InputImageType, OutputImageType > OtsuFilterType;

  typename OtsuFilterType::Pointer otsuFilter = OtsuFilterType::New();
  otsuFilter->SetInput( input );
  otsuFilter->SetNumberOfThresholds( m_NumberOfRegions - 1);
//  otsuFilter->ReleaseDataFlagOn();

  if (m_ProgressCommand.IsNotNull())
  {
    m_ProgressCommand->AddStepsToDo(200);
    otsuFilter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);
  }

  otsuFilter->Update();

  if (m_ProgressCommand.IsNotNull())
  {
    m_ProgressCommand->Reset();
  }

  typename OutputImageType::Pointer result = otsuFilter->GetOutput();
  result->DisconnectPipeline();

  m_MultiLabelImage = mitk::Image::New();
  mitk::CastToMitkImage(result.GetPointer(), m_MultiLabelImage);

  m_MultiLabelNode->SetData(m_MultiLabelImage);

  m_RequestedRegion = m_MultiLabelImage->GetLargestPossibleRegion();
}
