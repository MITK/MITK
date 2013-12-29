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
#include "mitkLabelSetImage.h"
#include "mitkRenderingManager.h"
#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"

// itk
#include <itkImageRegionIterator.h>
#include <itkBinaryThresholdImageFilter.h>

// us
#include "usModule.h"
#include "usModuleResource.h"
#include "usGetModuleContext.h"
#include "usModuleContext.h"

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, BinaryThresholdULTool, "BinaryThresholdULTool tool");
}

mitk::BinaryThresholdULTool::BinaryThresholdULTool() : SegTool3D("dummy"),
m_SensibleMinimumThresholdValue(-100),
m_SensibleMaximumThresholdValue(+100),
m_CurrentLowerThresholdValue(1),
m_CurrentUpperThresholdValue(100),
m_IsFloatImage(false)
{
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
  return "Two Thresholds";
}

void mitk::BinaryThresholdULTool::Activated()
{
  Superclass::Activated();

  m_PreviewNode->SetProperty("outline binary", BoolProperty::New(false) );
  m_PreviewNode->SetOpacity(0.6);

  bool referenceHasChanged(m_ReferenceNode != m_ToolManager->GetReferenceData(0));

  m_ReferenceNode = m_ToolManager->GetReferenceData(0);
  assert(m_ReferenceNode);

  Image* referenceImage = dynamic_cast<Image*> (m_ReferenceNode->GetData());
  assert(referenceImage);

  if ((referenceImage->GetPixelType().GetPixelType() == itk::ImageIOBase::SCALAR)
    && (referenceImage->GetPixelType().GetComponentType() == itk::ImageIOBase::FLOAT))
      m_IsFloatImage = true;
  else
      m_IsFloatImage = false;

  if (referenceHasChanged)
  {
    CurrentlyBusy.Send(true);

    m_SensibleMinimumThresholdValue = static_cast<mitk::ScalarType>( referenceImage->GetScalarValueMin() );
    m_SensibleMaximumThresholdValue = static_cast<mitk::ScalarType>( referenceImage->GetScalarValueMaxNoRecompute() );
    m_CurrentLowerThresholdValue = (m_SensibleMaximumThresholdValue + m_SensibleMinimumThresholdValue) / 3;
    m_CurrentUpperThresholdValue = 2*m_CurrentLowerThresholdValue;

    CurrentlyBusy.Send(false);
  }

  IntervalBordersChanged.Send(m_SensibleMinimumThresholdValue, m_SensibleMaximumThresholdValue, m_IsFloatImage);
  ThresholdingValuesChanged.Send(m_CurrentLowerThresholdValue, m_CurrentUpperThresholdValue);

  this->Run();
}

void mitk::BinaryThresholdULTool::SetThresholdValues(mitk::ScalarType lower, mitk::ScalarType upper)
{
  m_CurrentLowerThresholdValue = lower;
  m_CurrentUpperThresholdValue = upper;
}

void mitk::BinaryThresholdULTool::Run()
{
//  this->InitializeUndoController();

  Image* referenceImage = dynamic_cast< Image* >( m_ReferenceNode->GetData() );
  assert(referenceImage);

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
    MITK_ERROR << "Unknown exception caught!";
    m_ToolManager->ActivateTool(-1);
    return;
  }

  CurrentlyBusy.Send(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

template <typename TPixel, unsigned int VDimension>
void mitk::BinaryThresholdULTool::InternalRun( itk::Image<TPixel, VDimension>* input)
{
  typedef itk::Image<TPixel, VDimension> ImageType;
  typedef itk::Image< LabelSetImage::PixelType, VDimension> SegmentationType;
  typedef itk::BinaryThresholdImageFilter <ImageType, SegmentationType>  BinaryThresholdImageFilterType;

  BinaryThresholdImageFilterType::Pointer thresholdFilter = BinaryThresholdImageFilterType::New();
  thresholdFilter->SetInput(input);
  thresholdFilter->SetLowerThreshold(m_CurrentLowerThresholdValue);
  thresholdFilter->SetUpperThreshold(m_CurrentUpperThresholdValue);
  thresholdFilter->SetInsideValue(1);
  thresholdFilter->SetOutsideValue(0);

  thresholdFilter->Update();

  typename SegmentationType::Pointer result = thresholdFilter->GetOutput();
  result->DisconnectPipeline();

  m_PreviewImage = mitk::Image::New();
  mitk::CastToMitkImage(result, m_PreviewImage);

  m_PreviewNode->SetData(m_PreviewImage);
  m_PreviewNode->SetVisibility(true);

  m_RequestedRegion = m_PreviewImage->GetLargestPossibleRegion();
}
