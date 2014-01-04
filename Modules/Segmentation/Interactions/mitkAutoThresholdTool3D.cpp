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

#include "mitkAutoThresholdTool3D.h"

#include "mitkToolManager.h"
#include "mitkLabelSetImage.h"
#include "mitkRenderingManager.h"
#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"

// itk
#include <itkImageRegionIterator.h>
#include <itkHuangThresholdImageFilter.h>
#include <itkIntermodesThresholdImageFilter.h>
#include <itkIsoDataThresholdImageFilter.h>
#include <itkLiThresholdImageFilter.h>
#include <itkMaximumEntropyThresholdImageFilter.h>
#include <itkKittlerIllingworthThresholdImageFilter.h>
#include <itkMomentsThresholdImageFilter.h>
#include <itkOtsuThresholdImageFilter.h>
#include <itkRenyiEntropyThresholdImageFilter.h>
#include <itkShanbhagThresholdImageFilter.h>
#include <itkTriangleThresholdImageFilter.h>
#include <itkYenThresholdImageFilter.h>

// us
#include "usModule.h"
#include "usModuleResource.h"
#include "usGetModuleContext.h"
#include "usModuleContext.h"

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, AutoThresholdTool3D, "AutoThresholdTool3D tool");
}

mitk::AutoThresholdTool3D::AutoThresholdTool3D() : SegTool3D("dummy")
{
}

mitk::AutoThresholdTool3D::~AutoThresholdTool3D()
{
}

const char** mitk::AutoThresholdTool3D::GetXPM() const
{
  return NULL;
}

us::ModuleResource mitk::AutoThresholdTool3D::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("AutoThreshold_48x48.png");
  return resource;
}

const char* mitk::AutoThresholdTool3D::GetName() const
{
  return "Auto Threshold";
}

void mitk::AutoThresholdTool3D::SetMethod(AutoThresholdType value)
{
  m_ActiveMethod = value;
}

void mitk::AutoThresholdTool3D::Run()
{
//  this->InitializeUndoController();

  m_PreviewNode->SetProperty("outline binary", BoolProperty::New(false) );
  m_PreviewNode->SetOpacity(0.6);

  m_ReferenceNode = m_ToolManager->GetReferenceData(0);
  assert(m_ReferenceNode);

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
void mitk::AutoThresholdTool3D::InternalRun( itk::Image<TPixel, VDimension>* input)
{
  typedef itk::Image<TPixel, VDimension> ImageType;
  typedef itk::Image< LabelSetImage::PixelType, VDimension> SegmentationType;
  typedef itk::HuangThresholdImageFilter <ImageType, SegmentationType>  HuangThresholdImageFilterType;
  typedef itk::IntermodesThresholdImageFilter <ImageType, SegmentationType>  IntermodesThresholdImageFilterType;
  typedef itk::IsoDataThresholdImageFilter <ImageType, SegmentationType>  IsoDataThresholdImageFilterType;
  typedef itk::LiThresholdImageFilter <ImageType, SegmentationType>  LiThresholdImageFilterType;
  typedef itk::KittlerIllingworthThresholdImageFilter <ImageType, SegmentationType>  KittlerIllingworthThresholdImageFilterType;
  typedef itk::MaximumEntropyThresholdImageFilter <ImageType, SegmentationType>  MaximumEntropyThresholdImageFilterType;
  typedef itk::MomentsThresholdImageFilter <ImageType, SegmentationType>  MomentsThresholdImageFilterType;
  typedef itk::OtsuThresholdImageFilter <ImageType, SegmentationType>  OtsuThresholdImageFilterType;
  typedef itk::RenyiEntropyThresholdImageFilter <ImageType, SegmentationType>  RenyiEntropyThresholdImageFilterType;
  typedef itk::ShanbhagThresholdImageFilter <ImageType, SegmentationType>  ShanbhagThresholdImageFilterType;
  typedef itk::TriangleThresholdImageFilter <ImageType, SegmentationType>  TriangleThresholdImageFilterType;
  typedef itk::YenThresholdImageFilter <ImageType, SegmentationType>  YenThresholdImageFilterType;

  typename itk::HistogramThresholdImageFilter<ImageType, SegmentationType>::Pointer filter;

  typename SegmentationType::Pointer result;

  switch(m_ActiveMethod)
  {
  case AT_HUANG:
    filter = HuangThresholdImageFilterType::New();
    break;
  case AT_INTERMODES:
    filter = IntermodesThresholdImageFilterType::New();
    break;
  case AT_ISODATA:
    filter = IsoDataThresholdImageFilterType::New();
    break;
  case AT_LI:
    filter = LiThresholdImageFilterType::New();
    break;
  case AT_MAXENTROPY:
    filter = MaximumEntropyThresholdImageFilterType::New();
    break;
  case AT_MINERROR:
    filter = KittlerIllingworthThresholdImageFilterType::New();
    break;
  case AT_MOMENTS:
    filter = MomentsThresholdImageFilterType::New();
    break;
  case AT_OTSU:
    filter = OtsuThresholdImageFilterType::New();
    break;
  case AT_RENYIENTROPY:
    filter = RenyiEntropyThresholdImageFilterType::New();
    break;
  case AT_SHANBHAG:
    filter = ShanbhagThresholdImageFilterType::New();
    break;
  case AT_TRIANGLE:
    filter = TriangleThresholdImageFilterType::New();
    break;
  case AT_YEN:
    filter = YenThresholdImageFilterType::New();
    break;
  }

  filter->SetInput(input);
  filter->SetInsideValue(1);
  filter->SetOutsideValue(0);

  filter->AddObserver( itk::AnyEvent(), m_ProgressCommand );
  m_ProgressCommand->AddStepsToDo(100);

  filter->Update();

  m_ProgressCommand->Reset();

  result = filter->GetOutput();
  result->DisconnectPipeline();

  m_PreviewImage = mitk::Image::New();
  mitk::CastToMitkImage(result, m_PreviewImage);

  m_PreviewNode->SetData(m_PreviewImage);
  m_PreviewNode->SetVisibility(true);

  m_RequestedRegion = m_PreviewImage->GetLargestPossibleRegion();
}
