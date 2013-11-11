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

#include "mitkErodeTool3D.h"

#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
#include "mitkInteractionConst.h"
#include "mitkApplyDiffImageOperation.h"
#include "mitkOperationEvent.h"
#include "mitkDiffImageApplier.h"
#include "mitkUndoController.h"
#include "mitkImageAccessByItk.h"
#include "mitkToolManager.h"
#include "mitkImageCast.h"
#include "mitkImageTimeSelector.h"

#include <itkBinaryThresholdImageFilter.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryErodeImageFilter.h>

#include <itkMedianImageFilter.h>
#include <itkLabelObject.h>
#include <itkLabelMap.h>
#include <itkLabelImageToLabelMapFilter.h>
#include <itkAutoCropLabelMapFilter.h>
#include <itkLabelMapToLabelImageFilter.h>

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, ErodeTool3D, "ErodeTool3D tool");
}


mitk::ErodeTool3D::ErodeTool3D()
{
}

mitk::ErodeTool3D::~ErodeTool3D()
{
}

const char** mitk::ErodeTool3D::GetXPM() const
{
  return NULL;//mitkErodeTool3D_xpm;
}

us::ModuleResource mitk::ErodeTool3D::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("ErodeTool3D_48x48.png");
  return resource;
}

const char* mitk::ErodeTool3D::GetName() const
{
  return "ErodeTool3D";
}

void mitk::ErodeTool3D::Run()
{
  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( workingNode->GetData() );
  assert(workingImage);

  // todo: use it later
  unsigned int timestep = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetTime()->GetPos();

  m_ProgressCommand = mitk::ToolCommand::New();

  CurrentlyBusy.Send(true);

  try
  {
    AccessByItk(workingImage, ITKProcessing);
  }
  catch( itk::ExceptionObject& e )
  {
   MITK_ERROR << "Exception caught: " << e.GetDescription();
   m_ProgressCommand->Reset();
   CurrentlyBusy.Send(false);
   return;
  }

  CurrentlyBusy.Send(false);

  workingImage->Modified();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


template < typename TPixel, unsigned int VDimension >
void mitk::ErodeTool3D::ITKProcessing( itk::Image< TPixel, VDimension>* input )
{
  typedef itk::Image<TPixel, VDimension> ImageType;
  typedef itk::LabelObject< TPixel, VDimension > LabelObjectType;
  typedef itk::LabelMap< LabelObjectType > LabelMapType;
  typedef itk::LabelImageToLabelMapFilter< ImageType, LabelMapType > Image2LabelMapType;
  typedef itk::AutoCropLabelMapFilter< LabelMapType > AutoCropType;
  typedef itk::LabelMapToLabelImageFilter< LabelMapType, ImageType > LabelMap2ImageType;
  typedef itk::BinaryBallStructuringElement<TPixel, VDimension> BallType;
  typedef itk::BinaryErodeImageFilter<ImageType, ImageType, BallType> ErodeFilterType;
  typedef itk::BinaryThresholdImageFilter< ImageType, ImageType > ThresholdFilterType;

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( workingNode->GetData() );
  assert(workingImage);

  int pixelValue = workingImage->GetActiveLabelIndex();

  typename ThresholdFilterType::Pointer thresholdFilter = ThresholdFilterType::New();
  thresholdFilter->SetInput(input);
  thresholdFilter->SetLowerThreshold(pixelValue);
  thresholdFilter->SetUpperThreshold(pixelValue);
  thresholdFilter->SetOutsideValue(0);
  thresholdFilter->SetInsideValue(pixelValue);

  typename Image2LabelMapType::Pointer image2label = Image2LabelMapType::New();
  image2label->SetInput(thresholdFilter->GetOutput());

  typename AutoCropType::Pointer autoCropFilter = AutoCropType::New();
  autoCropFilter->SetInput( image2label->GetOutput() );
  autoCropFilter->InPlaceOn();

  typename LabelMap2ImageType::Pointer label2image = LabelMap2ImageType::New();
  label2image->SetInput( autoCropFilter->GetOutput() );

  BallType ball;
  ball.SetRadius(1.0);
  ball.CreateStructuringElement();

  typename ErodeFilterType::Pointer erodeFilter = ErodeFilterType::New();
  erodeFilter->SetKernel(ball);
  erodeFilter->SetInput(label2image->GetOutput());
  erodeFilter->SetErodeValue(pixelValue);

  if (m_ProgressCommand.IsNotNull())
  {
    thresholdFilter->AddObserver( itk::AnyEvent(), m_ProgressCommand );
    autoCropFilter->AddObserver( itk::AnyEvent(), m_ProgressCommand );
    erodeFilter->AddObserver( itk::AnyEvent(), m_ProgressCommand );
    m_ProgressCommand->AddStepsToDo(100);
  }

  erodeFilter->Update();

  if (m_ProgressCommand.IsNotNull())
  {
    m_ProgressCommand->Reset();
  }

  typename ImageType::Pointer result = erodeFilter->GetOutput();
  result->DisconnectPipeline();

  typename ImageType::RegionType cropRegion;
  cropRegion = autoCropFilter->GetOutput()->GetLargestPossibleRegion();

  typedef itk::ImageRegionConstIterator< ImageType > SourceIteratorType;
  typedef itk::ImageRegionIterator< ImageType > TargetIteratorType;

  typename const ImageType::SizeType& cropSize = cropRegion.GetSize();
  typename const ImageType::IndexType& cropIndex = cropRegion.GetIndex();

  SourceIteratorType sourceIter( result, result->GetLargestPossibleRegion() );
  sourceIter.GoToBegin();

  TargetIteratorType targetIter( input, cropRegion );
  targetIter.GoToBegin();

  while ( !sourceIter.IsAtEnd() )
  {
    int targetValue = static_cast< int >( targetIter.Get() );
    int sourceValue = static_cast< int >( sourceIter.Get() );

    if ( (targetValue == pixelValue) || ( sourceValue && (!workingImage->GetLabelLocked(targetValue))) )
    {
      targetIter.Set( sourceValue );
    }

    ++sourceIter;
    ++targetIter;
  }
}
