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

#include "mitkOpenTool3D.h"

#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
#include "mitkInteractionConst.h"
#include "mitkImageAccessByItk.h"
#include "mitkToolManager.h"
#include "mitkImageCast.h"
#include "mitkImageTimeSelector.h"

#include <itkBinaryThresholdImageFilter.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryMorphologicalOpeningImageFilter.h>

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
  MITK_TOOL_MACRO(Segmentation_EXPORT, OpenTool3D, "OpenTool3D tool");
}


mitk::OpenTool3D::OpenTool3D()
{
  m_ProgressCommand = mitk::ToolCommand::New();
}

mitk::OpenTool3D::~OpenTool3D()
{
}

const char** mitk::OpenTool3D::GetXPM() const
{
  return NULL;//mitkOpenTool3D_xpm;
}

us::ModuleResource mitk::OpenTool3D::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  assert(module);
  us::ModuleResource resource = module->GetResource("OpenTool3D_48x48.png");
  return resource;
}

const char* mitk::OpenTool3D::GetName() const
{
  return "OpenTool3D";
}

void mitk::OpenTool3D::Run()
{
//  this->InitializeUndoController();

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( workingNode->GetData() );
  assert(workingImage);

  // todo: use it later
  //unsigned int timestep = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetTime()->GetPos();

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
void mitk::OpenTool3D::ITKProcessing( itk::Image< TPixel, VDimension>* input )
{
  typedef itk::Image<TPixel, VDimension> ImageType;
  typedef itk::LabelObject< TPixel, VDimension > LabelObjectType;
  typedef itk::LabelMap< LabelObjectType > LabelMapType;
  typedef itk::LabelImageToLabelMapFilter< ImageType, LabelMapType > Image2LabelMapType;
  typedef itk::AutoCropLabelMapFilter< LabelMapType > AutoCropType;
  typedef itk::LabelMapToLabelImageFilter< LabelMapType, ImageType > LabelMap2ImageType;
  typedef itk::BinaryBallStructuringElement<TPixel, VDimension> BallType;
  typedef itk::BinaryMorphologicalOpeningImageFilter<ImageType, ImageType, BallType> OpeningFilterType;
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

  typename AutoCropType::SizeType border;
  border[0] = 3;
  border[1] = 3;
  border[2] = 3;

  typename AutoCropType::Pointer autoCropFilter = AutoCropType::New();
  autoCropFilter->SetInput( image2label->GetOutput() );
  autoCropFilter->SetCropBorder(border);
  autoCropFilter->InPlaceOn();

  typename LabelMap2ImageType::Pointer label2image = LabelMap2ImageType::New();
  label2image->SetInput( autoCropFilter->GetOutput() );

  BallType ball;
  ball.SetRadius(1.0);
  ball.CreateStructuringElement();

  typename OpeningFilterType::Pointer openingFilter = OpeningFilterType::New();
  openingFilter->SetKernel(ball);
  openingFilter->SetInput(label2image->GetOutput());
  openingFilter->SetForegroundValue(pixelValue);
  openingFilter->SetBackgroundValue(0);

  if (m_ProgressCommand.IsNotNull())
  {
    thresholdFilter->AddObserver( itk::AnyEvent(), m_ProgressCommand );
    autoCropFilter->AddObserver( itk::AnyEvent(), m_ProgressCommand );
    openingFilter->AddObserver( itk::AnyEvent(), m_ProgressCommand );
    m_ProgressCommand->AddStepsToDo(200);
  }

  openingFilter->Update();

  if (m_ProgressCommand.IsNotNull())
  {
    m_ProgressCommand->Reset();
  }

  typename ImageType::Pointer result = openingFilter->GetOutput();
  result->DisconnectPipeline();

  m_PreviewImage = mitk::Image::New();
  m_PreviewImage->InitializeByItk(result.GetPointer());
  m_PreviewImage->SetChannel(result->GetBufferPointer());

  typename ImageType::RegionType cropRegion;
  cropRegion = autoCropFilter->GetOutput()->GetLargestPossibleRegion();

  const typename ImageType::SizeType& cropSize = cropRegion.GetSize();
  const typename ImageType::IndexType& cropIndex = cropRegion.GetIndex();

  mitk::SlicedGeometry3D* slicedGeometry = m_PreviewImage->GetSlicedGeometry();
  mitk::Point3D origin;
  vtk2itk(cropIndex, origin);
  workingImage->GetSlicedGeometry()->IndexToWorld(origin, origin);
  slicedGeometry->SetOrigin(origin);

  m_PreviewNode->SetData(m_PreviewImage);

  m_RequestedRegion = workingImage->GetLargestPossibleRegion();

  m_RequestedRegion.SetIndex(0,cropIndex[0]);
  m_RequestedRegion.SetIndex(1,cropIndex[1]);
  m_RequestedRegion.SetIndex(2,cropIndex[2]);

  m_RequestedRegion.SetSize(0,cropSize[0]);
  m_RequestedRegion.SetSize(1,cropSize[1]);
  m_RequestedRegion.SetSize(2,cropSize[2]);
}
