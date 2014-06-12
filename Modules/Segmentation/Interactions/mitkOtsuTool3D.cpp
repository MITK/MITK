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
#include "mitkRenderingModeProperty.h"
#include "mitkLevelWindowProperty.h"
#include "mitkLookupTableProperty.h"

// itk
#include <itkOtsuMultipleThresholdsImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkOrImageFilter.h>

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk {
  MITK_TOOL_MACRO(MitkSegmentation_EXPORT, OtsuTool3D, "Multiple Otsu Segmentation");
}

mitk::OtsuTool3D::OtsuTool3D() : SegTool3D("dummy")
{
}

mitk::OtsuTool3D::~OtsuTool3D()
{

}

void mitk::OtsuTool3D::Activated()
{
  Superclass::Activated();

  // feedback node and its visualization properties
  m_MultiLabelNode = mitk::DataNode::New();
  m_MultiLabelNode->SetName( "multilabel preview" );
  m_MultiLabelNode->SetOpacity(0.7);
  m_MultiLabelNode->SetColor(1.0, 1.0, 1.0);
  m_MultiLabelNode->SetProperty( "Image Rendering.Mode", RenderingModeProperty::New( RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR) );
  m_MultiLabelNode->SetProperty( "texture interpolation", BoolProperty::New(false) );
  m_MultiLabelNode->SetProperty( "layer", IntProperty::New(100) );
  m_MultiLabelNode->SetProperty( "binary", BoolProperty::New(false) );
  m_MultiLabelNode->SetProperty( "helper object", BoolProperty::New(true) );

  mitk::LookupTable::Pointer mitkLut = mitk::LookupTable::New();
  mitk::LookupTableProperty::Pointer mitkLutProp = mitk::LookupTableProperty::New();
  mitkLutProp->SetLookupTable(mitkLut);
  mitkLut->SetType(mitk::LookupTable::MULTILABEL);
  m_MultiLabelNode->SetProperty("LookupTable", mitkLutProp);

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

us::ModuleResource mitk::OtsuTool3D::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Otsu_48x48.png");
  return resource;
}


const char** mitk::OtsuTool3D::GetXPM() const
{
  return NULL;
}

void mitk::OtsuTool3D::RunSegmentation(int regions, bool useValley, int numberOfBins)
{
  m_ReferenceNode = m_ToolManager->GetReferenceData(0);
  assert(m_ReferenceNode);

  mitk::Image* referenceImage = dynamic_cast< mitk::Image* >( m_ReferenceNode->GetData() );
  assert(referenceImage);

  mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( m_WorkingNode->GetData() );
  assert(workingImage);

  m_PaintingPixelValue = workingImage->GetActiveLabel()->GetPixelValue();

  m_CurrentTimeStep = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetTime()->GetPos();

  //  this->InitializeUndoController();
  CurrentlyBusy.Send(true);

  try
  {
    AccessByItk_3(referenceImage, InternalRun, regions, useValley, numberOfBins);
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
  return "Multiple Otsu";
}

//const char* mitk::OtsuTool3D::GetName() const
//{
//  return "Otsu";
//}

void mitk::OtsuTool3D::UpdatePreview(std::vector<int> regionIDs)
{
  mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( m_WorkingNode->GetData() );
  assert(workingImage);

  m_RegionIDs = regionIDs;

  CurrentlyBusy.Send(true);

  try
  {
    // Fixed pixel type due to itk::OrFilter which is used for update preview
    AccessFixedPixelTypeByItk( m_MultiLabelImage, InternalUpdatePreview, (unsigned char) );
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

template <typename TPixel, unsigned int VImageDimension>
void mitk::OtsuTool3D::InternalUpdatePreview(const itk::Image<TPixel, VImageDimension>* input)
{
  typedef itk::Image< TPixel, VImageDimension > ImageType;
  typedef itk::BinaryThresholdImageFilter< ImageType, ImageType > ThresholdFilterType;

  typename ImageType::Pointer itkBinaryTempImage1;
  typename ImageType::Pointer itkBinaryTempImage2;
  typename ImageType::Pointer itkBinaryResultImage;

  typename ThresholdFilterType::Pointer filter = ThresholdFilterType::New();

  filter->SetInput(input);
  filter->SetLowerThreshold(m_RegionIDs[0]);
  filter->SetUpperThreshold(m_RegionIDs[0]);
  filter->SetInsideValue(1);
  filter->SetOutsideValue(0);

//  if (m_ProgressCommand.IsNotNull())
//  {
//    m_ProgressCommand->AddStepsToDo(200);
//    filter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);
//  }

//  filter->Update();

//  if (m_ProgressCommand.IsNotNull())
//  {
//    m_ProgressCommand->Reset();
//  }

  filter->Update();
  itkBinaryTempImage2 = filter->GetOutput();

  typename itk::OrImageFilter< ImageType, ImageType>::Pointer orFilter = itk::OrImageFilter< ImageType, ImageType>::New();

  // if more than one region id is used compute the union of all given binary regions
  for (std::vector<int>::iterator it = m_RegionIDs.begin() ; it != m_RegionIDs.end(); ++it)
  {
    filter->SetLowerThreshold(*it);
    filter->SetUpperThreshold(*it);
    filter->SetInsideValue(1);
    filter->SetOutsideValue(0);
    filter->Update();
    itkBinaryTempImage1 = filter->GetOutput();

    orFilter->SetInput1(itkBinaryTempImage1);
    orFilter->SetInput2(itkBinaryTempImage2);

    orFilter->UpdateLargestPossibleRegion();
    itkBinaryResultImage = orFilter->GetOutput();
    itkBinaryTempImage2 = itkBinaryResultImage;
  }
  //----------------------------------------------------------------------------------------------------
  itkBinaryResultImage->DisconnectPipeline();

  m_PreviewImage = mitk::Image::New();
  mitk::CastToMitkImage( itkBinaryResultImage, m_PreviewImage);
  m_PreviewNode->SetData(m_PreviewImage);
  m_PreviewNode->SetVisibility(true);
  m_PreviewNode->SetProperty("outline binary", mitk::BoolProperty::New(false));

  m_RequestedRegion = m_PreviewImage->GetLargestPossibleRegion();
}

template < typename TPixel, unsigned int VDimension >
void mitk::OtsuTool3D::InternalRun( itk::Image<TPixel, VDimension>* input, int regions, bool useValley, int numberOfBins )
{
  typedef itk::Image< TPixel, VDimension > InputImageType;
  typedef itk::Image< LabelSetImage::PixelType, VDimension > OutputImageType;
  typedef itk::OtsuMultipleThresholdsImageFilter< InputImageType, OutputImageType > OtsuFilterType;

  typename OtsuFilterType::Pointer otsuFilter = OtsuFilterType::New();
  otsuFilter->SetNumberOfThresholds( regions - 1 );
  otsuFilter->SetValleyEmphasis( useValley );
  otsuFilter->SetNumberOfHistogramBins( numberOfBins );
  otsuFilter->SetInput( input );

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
