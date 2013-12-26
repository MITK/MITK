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

#include "mitkMedianTool3D.h"

#include "mitkLabelSetImage.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
#include "mitkImageAccessByItk.h"
#include "mitkToolManager.h"
#include "mitkImageCast.h"

// itk
#include <itkBinaryThresholdImageFilter.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryMedianImageFilter.h>
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
  MITK_TOOL_MACRO(Segmentation_EXPORT, MedianTool3D, "MedianTool3D tool");
}

mitk::MedianTool3D::MedianTool3D() : m_Radius(1)
{
}

mitk::MedianTool3D::~MedianTool3D()
{
}

const char** mitk::MedianTool3D::GetXPM() const
{
  return NULL;//mitkMedianTool3D_xpm;
}

us::ModuleResource mitk::MedianTool3D::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("MedianTool3D_48x48.png");
  return resource;
}

const char* mitk::MedianTool3D::GetName() const
{
  return "Median";
}

void mitk::MedianTool3D::SetRadius(int value)
{
  m_Radius = value;
}

int mitk::MedianTool3D::GetRadius()
{
  return m_Radius;
}

void mitk::MedianTool3D::Run()
{
//  this->InitializeUndoController();

  mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( m_WorkingNode->GetData() );
  assert(workingImage);

  m_CurrentTimeStep = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetTime()->GetPos();

  m_OverwritePixelValue = workingImage->GetActiveLabelIndex();

  CurrentlyBusy.Send(true);

  try
  {
    AccessByItk(workingImage, InternalRun);
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


template < typename TPixel, unsigned int VDimension >
void mitk::MedianTool3D::InternalRun( itk::Image< TPixel, VDimension>* input )
{
  typedef itk::Image<TPixel, VDimension> ImageType;

  typedef itk::LabelObject< TPixel, VDimension > LabelObjectType;
  typedef itk::LabelMap< LabelObjectType > LabelMapType;
  typedef itk::LabelImageToLabelMapFilter< ImageType, LabelMapType > Image2LabelMapType;
  typedef itk::AutoCropLabelMapFilter< LabelMapType > AutoCropType;
  typedef itk::LabelMapToLabelImageFilter< LabelMapType, ImageType > LabelMap2ImageType;
  typedef itk::BinaryBallStructuringElement<TPixel, VDimension> BallType;
  typedef itk::BinaryThresholdImageFilter< ImageType, ImageType > ThresholdFilterType;
  typedef itk::BinaryMedianImageFilter< ImageType, ImageType > MedianFilterType;

  mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( m_WorkingNode->GetData() );
  assert(workingImage);

  typename ThresholdFilterType::Pointer thresholdFilter = ThresholdFilterType::New();
  thresholdFilter->SetInput(input);
  thresholdFilter->SetLowerThreshold(m_OverwritePixelValue);
  thresholdFilter->SetUpperThreshold(m_OverwritePixelValue);
  thresholdFilter->SetOutsideValue(0);
  thresholdFilter->SetInsideValue(1);

  typename Image2LabelMapType::Pointer image2label = Image2LabelMapType::New();
  image2label->SetInput(thresholdFilter->GetOutput());

  typename AutoCropType::SizeType border;
  border[0] = m_Radius+1;
  border[1] = m_Radius+1;
  border[2] = m_Radius+1;

  typename AutoCropType::Pointer autoCropFilter = AutoCropType::New();
  autoCropFilter->SetInput( image2label->GetOutput() );
  autoCropFilter->SetCropBorder(border);
  autoCropFilter->InPlaceOn();

  typename LabelMap2ImageType::Pointer label2image = LabelMap2ImageType::New();
  label2image->SetInput( autoCropFilter->GetOutput() );

  thresholdFilter->AddObserver( itk::AnyEvent(), m_ProgressCommand );
  autoCropFilter->AddObserver( itk::AnyEvent(), m_ProgressCommand );
  m_ProgressCommand->AddStepsToDo(100);
  label2image->Update();
  m_ProgressCommand->Reset();

  typename ImageType::SizeType radius;
  radius.Fill(m_Radius);

  typename MedianFilterType::Pointer medianFilter = MedianFilterType::New();
  medianFilter->SetInput( label2image->GetOutput() );
  medianFilter->SetForegroundValue( 1 );
  medianFilter->SetRadius( radius );

  medianFilter->AddObserver( itk::AnyEvent(), m_ProgressCommand );
  m_ProgressCommand->AddStepsToDo(100);

  medianFilter->Update();

  m_ProgressCommand->Reset();

  typename ImageType::Pointer result = medianFilter->GetOutput();
  result->DisconnectPipeline();

  // fix intersections with other labels
  typedef itk::ImageRegionConstIterator< ImageType > InputIteratorType;
  typedef itk::ImageRegionIterator< ImageType >      ResultIteratorType;

  typename ImageType::RegionType cropRegion;
  cropRegion = autoCropFilter->GetOutput()->GetLargestPossibleRegion();

  typename InputIteratorType  inputIter( input, cropRegion );
  typename ResultIteratorType resultIter( result, result->GetLargestPossibleRegion() );

  inputIter.GoToBegin();
  resultIter.GoToBegin();

  while ( !resultIter.IsAtEnd() )
  {
    int inputValue = static_cast<int>( inputIter.Get() );

    if ( (inputValue != m_OverwritePixelValue) && workingImage->GetLabelLocked( inputValue ) )
      resultIter.Set(0);

    ++inputIter;
    ++resultIter;
  }

  m_PreviewImage = mitk::Image::New();
  mitk::CastToMitkImage(result.GetPointer(), m_PreviewImage);

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
